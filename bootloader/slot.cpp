#include <bootloader/slot.h>
#include <ion/src/device/shared/drivers/board.h>
#include <bootloader/keys.h>
#include <ion.h>

extern "C" void jump_to_firmware(const uint32_t* stackPtr, const void(*startPtr)(void));

namespace Bootloader {

const Slot Slot::A() {
  return Slot(0x90000000);
}

const Slot Slot::B() {
  return Slot(0x90400000);
}

const KernelHeader* Slot::kernelHeader() const {
  return m_kernelHeader;
}

const UserlandHeader* Slot::userlandHeader() const {
  return m_userlandHeader;
}

[[ noreturn ]] void Slot::boot() const {
  // Configure the MPU for the booted firmware
  Ion::Device::Board::bootloaderMPU();

  // Jump
  jump_to_firmware(kernelHeader()->stackPointer(), kernelHeader()->startPointer());
  for(;;);
}

const uint8_t* Slot::signature() const {
  return ((uint8_t*) m_kernelHeader) + m_kernelHeader->m_signature + 8;
}

const uint8_t* Slot::signedPayload() const {
  return ((uint8_t*) m_kernelHeader) + 8;
}

const size_t Slot::signedPayloadLength() const {
  return m_kernelHeader->m_signature;
}

const Key* Slot::checkSign() const {
  if (signature() < (const uint8_t*) m_kernelHeader || signature() >= (const uint8_t*) (m_kernelHeader + 0x400000) || signedPayloadLength() > 0x400000) {
    return nullptr;
  }

  for(size_t j = 0; j < sizeof(AvailableKeys) / sizeof(Key); j++) {
    if (Ion::verify(signature(), signedPayload(), signedPayloadLength(), AvailableKeys[j].ed25519_key)) {
      return &AvailableKeys[j];
    }
  }

  return nullptr;
}

}
