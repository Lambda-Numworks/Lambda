#ifndef ION_DEVICE_SHARED_FILESYSTEM_H
#define ION_DEVICE_SHARED_FILESYSTEM_H

#include <stddef.h>
#include <stdint.h>

extern uint8_t _filesystem_buffer_start[];
extern uint8_t _filesystem_buffer_len[];
extern uint8_t _filesystem_buffer_blocks[];
extern uint8_t _filesystem_buffer_blocks_size[];

namespace Ion {
namespace Device {
namespace FileSystem {

namespace Config {

constexpr size_t k_pageSize = 256;

}

void init();
void shutdown();

}
}
}

#endif