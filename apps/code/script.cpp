#include <ion/filesystem.h>

#include "script.h"
#include "script_store.h"

namespace Code {

static inline void intToText(int i, char * buffer, int bufferSize) {
  // We only support integers from 0 to 99.
  assert(i >= 0);
  assert(i < 100);
  assert(bufferSize >= 3);
  if (i/10 == 0) {
    buffer[0] = i+'0';
    buffer[1] = 0;
    return;
  }
  buffer[0] = i/10+'0';
  buffer[1] = i-10*(i/10)+'0';
  buffer[2] = 0;
}

bool Script::DefaultName(char buffer[], size_t bufferSize) {
  assert(bufferSize >= k_defaultScriptNameMaxSize);
  static constexpr char defaultScriptName[] = "script";
  static constexpr int defaultScriptNameLength = 6;
  strlcpy(buffer, defaultScriptName, bufferSize);

  int currentScriptNumber = 1;
  while (currentScriptNumber <= k_maxNumberOfDefaultScriptNames) {
    // Change the number in the script name.
    intToText(currentScriptNumber, &buffer[defaultScriptNameLength], bufferSize - defaultScriptNameLength );
    if (ScriptStore::ScriptNameIsFree(buffer)) {
      return true;
    }
    currentScriptNumber++;
  }
  // We did not find a new script name
  return false;
}

bool Script::nameCompliant(const char * name) {
  /* We allow here the empty script name ".py", because it is the name used to
   * create a new empty script. When naming or renaming a script, we check
   * elsewhere that the name is no longer empty.
   * The name format is ([a-z_][a-z0-9_]*)*\.py
   *
   * We do not allow upper cases in the script names because script names are
   * used in the URLs of the NumWorks workshop website and we do not want
   * problems with case sensitivity. */
  UTF8Decoder decoder(name);
  CodePoint c = decoder.nextCodePoint();
  if (c == UCodePointNull || !(c.isLatinSmallLetter() || c == '_' || c == '.')) {
    /* The name cannot be empty. Its first letter must be in [a-z_] or the
     * extension dot. */
    return false;
  }
  while (c != UCodePointNull) {
    if (c == '.' && strcmp(decoder.stringPosition(), ScriptStore::k_scriptExtension) == 0) {
      return true;
    }
    if (!(c.isLatinSmallLetter() || c == '_' || c.isDecimalDigit())) {
      return false;
    }
    c = decoder.nextCodePoint();
  }
  return false;
}

char * Script::content(char* out_buffer) const {
  if (m_null)
    return NULL;

  spiffs_file fd = SPIFFS_open(&global_filesystem, m_name, SPIFFS_O_RDONLY, 0);
  if (fd < 0)
    return NULL;

  size_t size = 0;
  char* tmp = out_buffer;

  while(1) {
    int res = SPIFFS_read(&global_filesystem, fd, tmp, 256);
    if (res < 0)
      break;

    size += res;
    tmp = out_buffer + size;

    if (res == 0)
      break;
  }

  int res = SPIFFS_close(&global_filesystem, fd);
  assert(res >= 0);
  out_buffer[size] = '\0';
  return out_buffer;
}

void Script::save(char* out_buffer, size_t size) const {
  if (m_null)
    return;
  
  spiffs_file fd = SPIFFS_open(&global_filesystem, m_name, SPIFFS_O_CREAT | SPIFFS_O_WRONLY | SPIFFS_O_TRUNC | SPIFFS_O_DIRECT, 0);
  assert(fd > 0);
  if (fd < 0)
    return;
  
  size_t tmp_size = 0;
  char* tmp = out_buffer;

  while(1) {
    int to_write = (size - tmp_size) < 256 ? (size - tmp_size) : 256;
    int res = SPIFFS_write(&global_filesystem, fd, tmp, to_write);
    if (res < 0)
      break;

    tmp_size += res;
    tmp = out_buffer + tmp_size;

    if (res == 0)
      break;
  }

  int res = SPIFFS_close(&global_filesystem, fd);
  assert(res >= 0);
  if (res < 0)
    return;
}

size_t Script::contentSize() const {
  if (m_null)
    return 0;

  spiffs_stat info;

  int res = SPIFFS_stat(&global_filesystem, m_name, &info);
  assert(res >= 0);
  if (res < 0)
    return 0;
  
  return info.size;
}

void Script::destroy() {
  int res = SPIFFS_remove(&global_filesystem, m_name);
  assert(res >= 0);
  m_null = true;
}

Ion::Storage::Record::ErrorStatus Script::setBaseNameWithExtension(const char* newName, const char* extension) {
  char name_buffer[65];
  strncpy(name_buffer, newName, 64);
  strncat(name_buffer, ".", 64);
  strncat(name_buffer, extension, 64);

  return setName(name_buffer);
}

Ion::Storage::Record::ErrorStatus Script::setName(const char* newName) {
  int res = SPIFFS_rename(&global_filesystem, m_name, newName);

  if (res == SPIFFS_ERR_CONFLICTING_NAME)
    return Ion::Storage::Record::ErrorStatus::NameTaken;
  else if (res == SPIFFS_ERR_NAME_TOO_LONG || res == SPIFFS_ERR_FULL)
    return Ion::Storage::Record::ErrorStatus::NotEnoughSpaceAvailable;

  assert(res >= 0);

  strncpy(m_name, newName, 65);

  return Ion::Storage::Record::ErrorStatus::None;
}

}
