#ifndef ION_FILESYSTEM_H
#define ION_FILESYSTEM_H

#include <stdint.h>
#include <stdlib.h>

#include "../../ion/src/external/spiffs/spiffs.h"

extern "C" spiffs global_filesystem;

namespace Ion {
namespace FileSystem {

char * storageUsage(char* buffer);
char * storageSize(char* buffer);
void format();

}
}

#endif
