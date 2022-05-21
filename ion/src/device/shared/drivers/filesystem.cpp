
#include <assert.h>
#include <string.h>

#include "filesystem.h"
#include "flash.h"

#include <ion/display.h>
#include <ion/keyboard.h>
#include <ion/timing.h>
#include <ion/filesystem.h>

extern "C" spiffs global_filesystem;

namespace Ion {
namespace Device {
namespace FileSystem {

int32_t __attribute__ ((noinline)) flash_read(uint32_t addr, uint32_t size, uint8_t *dst) {
  memcpy(dst, (uint8_t *) addr, size);
  return SPIFFS_OK;
}

int32_t __attribute__ ((noinline)) flash_prog(uint32_t addr, uint32_t size, uint8_t *src) {
  Flash::WriteMemory((uint8_t *) addr, src, size);
  return SPIFFS_OK;
}

int32_t __attribute__ ((noinline)) flash_erase(uint32_t addr, uint32_t size) {
  for(int i = Flash::SectorAtAddress(addr); i < Flash::SectorAtAddress(addr + size); i++)
    Flash::EraseSector(i);
  return SPIFFS_OK;
}

static uint8_t s_workBuf[Config::k_pageSize*2];
static uint8_t s_fds[32*4];
static uint8_t s_cacheBuffer[(Config::k_pageSize+32)*4 + 40];

static const spiffs_config cfg = {
  .hal_read_f = flash_read,
  .hal_write_f = flash_prog,
  .hal_erase_f = flash_erase,
  .phys_size = (uint32_t) _filesystem_buffer_len,
  .phys_addr = (uint32_t) _filesystem_buffer_start,
  .phys_erase_block = (uint32_t) _filesystem_buffer_blocks_size,
  .log_block_size = (uint32_t) _filesystem_buffer_blocks_size,
  .log_page_size = Config::k_pageSize
};

void init() {
  int res = SPIFFS_mount(&global_filesystem, &cfg, s_workBuf, s_fds, sizeof(s_fds), s_cacheBuffer, sizeof(s_cacheBuffer), 0);

  if (res != 0) {
    Ion::Display::pushRectUniform(KDRect(0, 0, 320, 10), KDColorBlack);
    Ion::FileSystem::format([](int current, int total, void* _) {
      Ion::Display::pushRectUniform(KDRect(0, 0, (320 * current) / total, 10), KDColorGreen);
    });
  }
}

void shutdown() {
  SPIFFS_unmount(&global_filesystem);
}

}
}

namespace FileSystem {

void format(FormatCallback callback, void* sender) {
  SPIFFS_unmount(&global_filesystem);

  spiffs_block_ix bix = 0;
  while (bix < global_filesystem.block_count) {
    global_filesystem.max_erase_count = 0;
    spiffs_erase_block(&global_filesystem, bix);
    bix++;

    if (callback != nullptr)
      callback(bix, global_filesystem.block_count, sender);
  }

  int res = SPIFFS_mount(&global_filesystem, &Ion::Device::FileSystem::cfg, Ion::Device::FileSystem::s_workBuf, Ion::Device::FileSystem::s_fds, sizeof(Ion::Device::FileSystem::s_fds), Ion::Device::FileSystem::s_cacheBuffer, sizeof(Ion::Device::FileSystem::s_cacheBuffer), 0);
  assert (res == 0);
}

}

}
