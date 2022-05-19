#include <drivers/external_flash.h>
#include <drivers/cache.h>
#include <drivers/config/external_flash.h>
#include <drivers/config/clocks.h>
#include <drivers/trampoline.h>
#include <ion/timing.h>
#include <assert.h>

namespace Ion {
namespace Device {
namespace ExternalFlash {

using namespace Regs;

/* The external flash and the Quad-SPI peripheral support several operating
 * modes, corresponding to different numbers of signals used to communicate
 * during each phase of the command sequence.
 *
 *   Mode name for | Number of signals used during each phase:
 *  external flash | Instruction | Address | Alt. bytes | Data
 * ----------------+-------------+---------+------------+------
 * Standard    SPI |      1      |    1    |     1      |   1
 * Dual-Output SPI |      1      |    1    |     1      |   2
 * Dual-I/O    SPI |      1      |    2    |     2      |   2
 * Quad-Output SPI |      1      |    1    |     1      |   4
 * Quad-I/O    SPI |      1      |    4    |     4      |   4
 *             QPI |      4      |    4    |     4      |   4
 *
 * The external flash supports clock frequencies up to 104MHz for all
 * instructions, except for Read Data (0x03) which is supported up to 50Mhz.
 *
 *
 *                Quad-SPI block diagram
 *
 *               +----------------------+          +------------+
 *               |       Quad-SPI       |          |            |
 *               |      peripheral      |          |  External  |
 *               |                      |   read   |   flash    |
 *   AHB    <--  |   data   <-- 32-byte |   <--    |   memory   |
 *  matrix  -->  | register -->  FIFO   |    -->   |            |
 *               +----------------------+   write  +------------+
 *
 * Any data transmitted to or from the external flash memory go through a
 * 32-byte FIFO.
 *
 * Read or write operations are performed in burst mode, that is, after any data
 * byte is transmitted between the Quad-SPI and the flash memory, the latter
 * automatically increments the specified address and the next byte to read or
 * write is respectively pushed in or popped from the FIFO.
 * And so on, as long as the clock continues.
 *
 * If the FIFO gets full in a read operation or
 * if the FIFO gets empty in a write operation,
 * the operation stalls and CLK stays low until firmware services the FIFO.
 *
 * If the FIFO gets full in a write operation, the operation is stalled until
 * the FIFO has enough space to accept the amount of data being written.
 * If the FIFO does not have as many bytes as requested by the read operation
 * and if BUSY=1, the operation is stalled until enough data is present or until
 * the transfer is complete, whichever happens first. */

enum class Command : uint8_t {
  WriteStatusRegister     = 0x01,
  PageProgram             = 0x02, // Program previously erased memory areas as being "0"
  ReadData                = 0x03,
  ReadStatusRegister1     = 0x05,
  WriteEnable             = 0x06,
  Erase4KbyteBlock        = 0x20,
  WriteStatusRegister2    = 0x31,
  QuadPageProgramW25Q64JV = 0x32,
  QuadPageProgramAT25F641 = 0x33,
  ReadStatusRegister2     = 0x35,
  Erase32KbyteBlock       = 0x52,
  EnableReset             = 0x66,
  Reset                   = 0x99,
  ReadJEDECID             = 0x9F,
  ReleaseDeepPowerDown    = 0xAB,
  DeepPowerDown           = 0xB9,
  ChipErase               = 0xC7, // Erase the whole chip or a 64-Kbyte block as being "1"
  Erase64KbyteBlock       = 0xD8,
  FastReadQuadIO          = 0xEB
};

static constexpr uint8_t NumberOfAddressBitsInBlock = 12;

class ExternalFlashStatusRegister {
public:
  class StatusRegister1 : public Register8 {
  public:
    using Register8::Register8;
    REGS_BOOL_FIELD_R(BUSY, 0);
  };
  class StatusRegister2 : public Register8 {
  public:
    using Register8::Register8;
    REGS_BOOL_FIELD(QE, 1);
  };
};

class OperatingModes {
public:
  constexpr OperatingModes(
      QUADSPI::CCR::OperatingMode instruction,
      QUADSPI::CCR::OperatingMode address,
      QUADSPI::CCR::OperatingMode data) :
    m_instructionOperatingMode(instruction),
    m_addressOperatingMode(address),
    m_dataOperatingMode(data)
  {}
  QUADSPI::CCR::OperatingMode instructionOperatingMode() const { return m_instructionOperatingMode; }
  QUADSPI::CCR::OperatingMode addressOperatingMode() const { return m_addressOperatingMode; }
  QUADSPI::CCR::OperatingMode dataOperatingMode() const { return m_dataOperatingMode; }
private:
  QUADSPI::CCR::OperatingMode m_instructionOperatingMode;
  QUADSPI::CCR::OperatingMode m_addressOperatingMode;
  QUADSPI::CCR::OperatingMode m_dataOperatingMode;
};

/* W25Q64JV does not implement QPI-4-4-4, so we always send the instructions on
 * one wire only.*/
static constexpr OperatingModes sOperatingModes100(QUADSPI::CCR::OperatingMode::Single, QUADSPI::CCR::OperatingMode::NoData, QUADSPI::CCR::OperatingMode::NoData);
static constexpr OperatingModes sOperatingModes101(QUADSPI::CCR::OperatingMode::Single, QUADSPI::CCR::OperatingMode::NoData, QUADSPI::CCR::OperatingMode::Single);
static constexpr OperatingModes sOperatingModes110(QUADSPI::CCR::OperatingMode::Single, QUADSPI::CCR::OperatingMode::Single, QUADSPI::CCR::OperatingMode::NoData);
static constexpr OperatingModes sOperatingModes111(QUADSPI::CCR::OperatingMode::Single, QUADSPI::CCR::OperatingMode::Single, QUADSPI::CCR::OperatingMode::Single);
static constexpr OperatingModes sOperatingModes114(QUADSPI::CCR::OperatingMode::Single, QUADSPI::CCR::OperatingMode::Single, QUADSPI::CCR::OperatingMode::Quad);
static constexpr OperatingModes sOperatingModes144(QUADSPI::CCR::OperatingMode::Single, QUADSPI::CCR::OperatingMode::Quad, QUADSPI::CCR::OperatingMode::Quad);

static constexpr int ClockFrequencyDivisor = 2; // F(QUADSPI) = F(AHB) / ClockFrequencyDivisor
static constexpr int FastReadQuadIODummyCycles = 4; // Must be 4 for W25Q64JV (Fig 24.A page 34) and for AT25F641 (table 7.19 page 28)
/* According to datasheets, the CS signal should stay high (deselect the device)
 * for t_SHSL = 50ns at least.
 * -> Max of 30ns (see AT25F641 Sections 8.7 and 8.8),
 *           10ns and 50ns (see W25Q64JV Section 9.6). */
static constexpr float ChipSelectHighTimeInNanoSeconds = 50.0f;

void init() {
}

void shutdown() {
}

int SectorAtAddress(uint32_t address) {
  int i = address >> NumberOfAddressBitsInBlock;
  assert(i <= Config::NumberOfSectors);
  return i;
}

void unlockFlash() {
  assert(false);
}

void MassErase() {
  // Mass erase is not enabled on kernel
  assert(false);
}

void WriteMemory(uint8_t * destination, const uint8_t * source, size_t length) {
  asm("cpsid if");
  (*reinterpret_cast<void(**)(uint8_t*, const uint8_t*, size_t)>(Ion::Device::Trampoline::address(Ion::Device::Trampoline::ExternalFlashWriteMemory)))(destination, source, length);
  asm("cpsie if");
}

void EraseSector(int i) {
  asm("cpsid if");
  (*reinterpret_cast<void(**)(int)>(Ion::Device::Trampoline::address(Ion::Device::Trampoline::ExternalFlashEraseSector)))(i);
  asm("cpsie if");
}

}
}
}
