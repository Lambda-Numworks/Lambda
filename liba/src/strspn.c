#include <string.h>

size_t strspn(const char *str, const char *chars) {
  size_t i = 0;
  while (str[i] && strchr(chars, str[i]))
    i++;
  return i;
}
