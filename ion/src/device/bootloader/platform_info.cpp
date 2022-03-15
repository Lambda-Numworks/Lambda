#include <ion.h>
#include <assert.h>

#ifndef PATCH_LEVEL
#error This file expects PATCH_LEVEL to be defined
#endif

#ifndef EPSILON_VERSION
#error This file expects EPSILON_VERSION to be defined
#endif

#ifndef HEADER_SECTION
#define HEADER_SECTION
#endif

namespace Ion {
extern char staticStorageArea[];
}
constexpr void * storageAddress = &(Ion::staticStorageArea);

class KernelHeader {
public:
  constexpr KernelHeader() :
    m_header(Magic),
    m_version{EPSILON_VERSION},
    m_patchLevel{PATCH_LEVEL},
    m_footer(Magic) { }
  const char * version() const {
    assert(m_header == Magic);
    assert(m_footer == Magic);
    return m_version;
  }
  const char * patchLevel() const {
    assert(m_header == Magic);
    assert(m_footer == Magic);
    return m_patchLevel;
  }
private:
  constexpr static uint32_t Magic = 0xDEC00DF0;
  uint32_t m_header;
  const char m_version[8];
  const char m_patchLevel[8];
  uint32_t m_footer;
};

const KernelHeader __attribute__((section(".kernel_header"), used)) k_kernelHeader;

class UserlandHeader {
public:
  constexpr UserlandHeader():
    m_header(Magic),
    m_expectedEpsilonVersion{EPSILON_VERSION},
    m_storageAddressRAM(storageAddress),
    m_storageSizeRAM(Ion::Storage::k_storageSize),
    m_externalAppsFlashStart(0xFFFFFFFF),
    m_externalAppsFlashEnd(0xFFFFFFFF),
    m_externalAppsRAMStart(0xFFFFFFFF),
    m_externalAppsRAMEnd(0xFFFFFFFF),
    m_footer(Magic) { }

private:
  constexpr static uint32_t Magic = 0xDEC0EDFE;
  constexpr static uint32_t OmegaMagic = 0xEFBEADDE;
  uint32_t m_header;
  const char m_expectedEpsilonVersion[8];
  void * m_storageAddressRAM;
  size_t m_storageSizeRAM;
  /* We store the range addresses of external apps memory because storing the
   * size is complicated due to c++11 constexpr. */
  uint32_t m_externalAppsFlashStart;
  uint32_t m_externalAppsFlashEnd;
  uint32_t m_externalAppsRAMStart;
  uint32_t m_externalAppsRAMEnd;
  uint32_t m_footer;
};

const UserlandHeader __attribute__((section(".userland_header"), used)) k_userlandHeader;

class SlotInfo {

public:
  SlotInfo() :
    m_header(Magic),
    m_footer(Magic) {}
  
  void update() {
    m_header = Magic;
    m_kernelHeaderAddress = &k_kernelHeader;
    m_userlandHeaderAddress = &k_userlandHeader;
    m_footer = Magic;
  }

private:
  constexpr static uint32_t Magic = 0xEFEEDBBA;
  uint32_t m_header;
  const KernelHeader * m_kernelHeaderAddress;
  const UserlandHeader * m_userlandHeaderAddress;
  uint32_t m_footer;

};

const char * Ion::softwareVersion() {
  return k_kernelHeader.version();
}

const char * Ion::patchLevel() {
  return k_kernelHeader.patchLevel();
}

SlotInfo * slotInfo() {
  static SlotInfo __attribute__((used)) __attribute__((section(".slot_info"))) slotInformation;
  return &slotInformation;
}

void Ion::updateSlotInfo() {
  slotInfo()->update();
}
