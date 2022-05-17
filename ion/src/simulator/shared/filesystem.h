#ifndef ION_SIMULATOR_FILESYSTEM_H
#define ION_SIMULATOR_FILESYSTEM_H

#include <stdint.h>

#include <ion.h>

namespace Ion {
namespace Simulator {
namespace FileSystem {

void init();

namespace Config {

constexpr size_t k_totalSize = 8 * 1024 * 1024;
constexpr size_t k_pageSize = 256;
constexpr size_t k_blockSize = 4 * 1024;

}

}
}
}

#endif
