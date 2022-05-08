#include <string.h>

size_t strnlen(const char * s, size_t max) {
  size_t i;

  for(i = 0; i < max; ++i)
    if (s[i] == '\0')
      break;

  return i;
}
