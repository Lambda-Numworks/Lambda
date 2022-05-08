#include <string.h>

char* strncpy(char * dst, const char * src, size_t dstSize) {
  const size_t srcLen = strnlen(src, dstSize);

  if (srcLen != dstSize)
    memset(dst+srcLen, '\0', dstSize - srcLen);

  return memcpy(dst, src, srcLen);
}
