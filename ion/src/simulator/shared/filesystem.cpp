
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <signal.h>
#include "filesystem.h"

#include "../../external/spiffs/spiffs.h"

extern "C" spiffs global_filesystem;

namespace Ion {
namespace Simulator {
namespace FileSystem {

uint8_t filesystem_buffer[Config::k_totalSize];

int32_t ram_read(uint32_t addr, uint32_t size, uint8_t *dst) {
  memcpy(dst, &filesystem_buffer[addr], size);
  return SPIFFS_OK;
}

int32_t ram_prog(uint32_t addr, uint32_t size, uint8_t *src) {
  for(size_t i = 0; i < size; i++) {
    filesystem_buffer[addr+i] &= src[i];
  }
  return SPIFFS_OK;
}

int32_t ram_erase(uint32_t addr, uint32_t size) {
  memset(&filesystem_buffer[addr], 0xFF, size);
  return SPIFFS_OK;
}

static uint8_t s_workBuf[Config::k_pageSize*2];
static uint8_t s_fds[32*4];
static uint8_t s_cacheBuffer[(Config::k_pageSize+32)*4 + 40];

static const spiffs_config cfg = {
  .hal_read_f = ram_read,
  .hal_write_f = ram_prog,
  .hal_erase_f = ram_erase,
  .phys_size = Config::k_totalSize,
  .phys_addr = 0,
  .phys_erase_block = Config::k_blockSize,
  .log_block_size = Config::k_blockSize,
  .log_page_size = Config::k_pageSize
};

void init() {
  memset(filesystem_buffer, 0xFF, Config::k_totalSize);
  
  int res = SPIFFS_mount(&global_filesystem, &cfg, s_workBuf, s_fds, sizeof(s_fds), s_cacheBuffer, sizeof(s_cacheBuffer), 0);

  if (res != 0) {
    Ion::FileSystem::format();
  }
}

}
}

namespace FileSystem {

void format() {
  SPIFFS_unmount(&global_filesystem);
  int res = SPIFFS_format(&global_filesystem);
  assert (res == 0);
  res = SPIFFS_mount(&global_filesystem, &Ion::Simulator::FileSystem::cfg, Ion::Simulator::FileSystem::s_workBuf, Ion::Simulator::FileSystem::s_fds, sizeof(Ion::Simulator::FileSystem::s_fds), Ion::Simulator::FileSystem::s_cacheBuffer, sizeof(Ion::Simulator::FileSystem::s_cacheBuffer), 0);
  assert (res == 0);
}

}

}
