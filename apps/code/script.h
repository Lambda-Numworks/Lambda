#ifndef CODE_SCRIPT_H
#define CODE_SCRIPT_H

#include <ion.h>

namespace Code {

/* Record: | Size |  Name |           Content              |
 *
 *
 *                           |FetchedForVariableBoxBit
 * Status is one byte long: xxxxxxxx
 *                          ^      ^
 *      FetchedFromConsoleBit      AutoImportationBit
 *
 * AutoImportationBit is 1 if the script should be auto imported when the
 * console opens.
 *
 * FetchedFromConsoleBit is 1 if its content has been fetched from the console,
 * so we can retrieve the correct variables afterwards in the variable box.
 *
 * FetchedForVariableBoxBit is used to prevent circular importation problems,
 * such as scriptA importing scriptB, which imports scriptA. Once we get the
 * variables from a script to put them in the variable box, we switch the bit to
 * 1 and won't reload it afterwards. */

class Script {
private:
  // Default script names are chosen between script1 and script99
  static constexpr int k_maxNumberOfDefaultScriptNames = 99;
  static constexpr int k_defaultScriptNameNumberMaxSize = 2; // Numbers from 1 to 99 have 2 digits max

public:
  static constexpr int k_defaultScriptNameMaxSize = 6 + k_defaultScriptNameNumberMaxSize + 1;

  static bool DefaultName(char buffer[], size_t bufferSize);
  static bool nameCompliant(const char * name);

  Script(char* name) : m_null(false) {
    strncpy(m_name, name, 65);
    m_name[64] = '\0';
  }

  Script() : m_null(true) {}

  char * content(char* buffer) const;
  void save(char* buffer, size_t size) const;
  char * fullName() { return m_name; }
  size_t contentSize() const;
  bool isNull() { return m_null; }
  void destroy();
  Ion::Storage::Record::ErrorStatus setName(const char* newName);
  Ion::Storage::Record::ErrorStatus setBaseNameWithExtension(const char* newName, const char* extension);

private:
  char m_name[65];
  bool m_null;
};

}

#endif
