#include <string.h>

char *strncat(char * dest, const char * src, size_t n) {
  int i, j;

  for (i = 0; dest[i] != '\0'; i++);

  for (j = 0; src[j] != '\0' && j < n; j++) {
    dest[i + j] = src[j];
  }

  dest[i + j] = '\0';

  return dest;
}
