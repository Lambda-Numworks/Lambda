#include <string.h>

char* strcpy(char * dst, const char * src) {
  const size_t srcLen = strlen(src);

  return memcpy(dst, src, srcLen);
}
