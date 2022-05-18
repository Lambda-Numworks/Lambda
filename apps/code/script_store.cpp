#include <assert.h>

#include <ion/filesystem.h>

#include "script_store.h"

namespace Code {

constexpr char ScriptStore::k_scriptExtension[];

bool ScriptStore::ScriptNameIsFree(const char * baseName) {
  char name_buffer[65] = "";
  strncat(name_buffer, baseName, 64);
  strncat(name_buffer, ".", 64);
  strncat(name_buffer, k_scriptExtension, 64);

  spiffs_stat stat;
  int out = SPIFFS_stat(&global_filesystem, name_buffer, &stat);
  assert (out == 0 | out == SPIFFS_ERR_NOT_FOUND);
  return out == SPIFFS_ERR_NOT_FOUND;
}

ScriptStore::ScriptStore() : m_scriptCount(0), m_scriptCountValid(false) {
  // addScriptFromTemplate(ScriptTemplate::Squares());
  // addScriptFromTemplate(ScriptTemplate::Parabola());
  // addScriptFromTemplate(ScriptTemplate::Mandelbrot());
  // addScriptFromTemplate(ScriptTemplate::Polynomial());
}

void ScriptStore::deleteAllScripts() {
  spiffs_DIR dir;
  struct spiffs_dirent entry;
  struct spiffs_dirent *p_entry = &entry;

  SPIFFS_opendir(&global_filesystem, "/", &dir);
  while((p_entry = SPIFFS_readdir(&dir, p_entry))) {
    if (Ion::Storage::FullNameHasExtension((char*) p_entry->name, k_scriptExtension, k_scriptExtensionLength)) {
      int fd = SPIFFS_open_by_dirent(&global_filesystem, p_entry, SPIFFS_RDWR, 0);
      if (fd < 0)
        break;
      int res = SPIFFS_fremove(&global_filesystem, fd);
      if (res < 0)
        break;
      res = SPIFFS_close(&global_filesystem, fd);
      if (res < 0)
        break;
    }
  }
  SPIFFS_closedir(&dir);
  m_scriptCountValid = false;
}

bool ScriptStore::isFull() {
  uint32_t total, used;
  SPIFFS_info(&global_filesystem, &total, &used);

  return used + 128 > total;
}

Ion::Storage::Record::ErrorStatus ScriptStore::addScriptFromTemplate(const ScriptTemplate * scriptTemplate) {
  m_scriptCountValid = false;
  size_t valueSize = strlen(scriptTemplate->content());
  if (!Script::nameCompliant(scriptTemplate->name()))
    return Ion::Storage::Record::ErrorStatus::NonCompliantName;
  
  spiffs_file fd = SPIFFS_open(&global_filesystem,scriptTemplate->name(), SPIFFS_O_CREAT | SPIFFS_O_WRONLY | SPIFFS_O_TRUNC, 0);
  if (fd == SPIFFS_ERR_CONFLICTING_NAME)
    return Ion::Storage::Record::ErrorStatus::NameTaken;

  assert(fd > 0);
  if (fd < 0)
    return Ion::Storage::Record::ErrorStatus::NonCompliantName;
  
  int res = SPIFFS_write(&global_filesystem, fd, (void*) scriptTemplate->value(), valueSize);

  assert(res >= 0);
  if (res < 0)
    return Ion::Storage::Record::ErrorStatus::NonCompliantName;
  
  res = SPIFFS_close(&global_filesystem, fd);

  assert(res >= 0);
  if (res < 0)
    return Ion::Storage::Record::ErrorStatus::NonCompliantName;

  return Ion::Storage::Record::ErrorStatus::None;
}


int ScriptStore::numberOfScripts() {
  if (m_scriptCountValid) {
    return m_scriptCount;
  } else {
    int out = 0;
    spiffs_DIR dir;
    struct spiffs_dirent entry;
    SPIFFS_opendir(&global_filesystem, "/", &dir);
    while(SPIFFS_readdir(&dir, &entry)) {
      if (Ion::Storage::FullNameHasExtension((char*) entry.name, k_scriptExtension, k_scriptExtensionLength)) {
        out++;
      }
    }
    SPIFFS_closedir(&dir);
    m_scriptCount = out;
    m_scriptCountValid = true;
    return out;
  }
}

Script ScriptStore::scriptAtIndex(int index) {
  Script out;
  spiffs_DIR dir;
  struct spiffs_dirent entry;
  int i = 0;

  SPIFFS_opendir(&global_filesystem, "/", &dir);
  while(( SPIFFS_readdir(&dir, &entry))) {
    if (Ion::Storage::FullNameHasExtension((char*) entry.name, k_scriptExtension, k_scriptExtensionLength)) {
      if (i == index) {
        out = Script((char*) entry.name);
        break;
      }
      i++;
    }
  }
  SPIFFS_closedir(&dir);
  return out;
}

}
