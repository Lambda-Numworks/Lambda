
#include <ion/filesystem.h>

#include <assert.h>
#include <string.h>

static void reverse(char s[]) {
  int i, j;
  char c;

  for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

static char* itoa(int n, char s[]) {
  int i, sign;

  if ((sign = n) < 0)
    n = -n;
  i = 0;
  do {
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  if (sign < 0)
    s[i++] = '-';

  s[i] = '\0';

  reverse(s);

  return s + i;
}

static char* human_size(size_t size, char* buffer) {
  const char* suffix = "B\0\0KB\0MB\0GB";

  size_t i;
  for(i = 0; size >= 1024; i++)
    size >>= 10;
  
  buffer = itoa(size, buffer);
  strcpy(buffer, suffix + i * 3);
  return buffer + strlen(suffix + i * 3);
}

namespace Ion {
namespace FileSystem {

char* storageUsage(char* buffer) {
  uint32_t total = 100, used = 0;

  SPIFFS_info(&global_filesystem, &total, &used);

  uint32_t percent = (used * 100) / total;
  char* tmp = itoa(percent, buffer);
  tmp[0] = '%';
  tmp[1] = '\0';

  return buffer;
}

char* storageSize(char* buffer) {
  uint32_t total = 100, used = 0;

  SPIFFS_info(&global_filesystem, &total, &used);

  char* buf = human_size(used, buffer);
  buf[0] = '/';
  buf = human_size(total, buf + 1);

  return buffer;
}

}
}
