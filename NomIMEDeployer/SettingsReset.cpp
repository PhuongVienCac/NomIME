#include "stdafx.h"
#include "SettingsReset.h"
#include <NomIMEConstants.h>
#include <NomIMEUtility.h>
#include <filesystem>
#include <string>
#include <vector>

namespace {

// Registry value holding the version that wrote the settings currently on
// disk. Missing means they predate this mechanism, i.e. 1.0.6 or earlier.
const wchar_t kSettingsVersionValue[] = L"SettingsVersion";

// Settings sit in the Rime user folder right next to the user's own data, so
// the reset names what it removes instead of clearing the folder.
//
// Removed -- written by NomIME's settings dialogs or by Rime on their behalf:
//   *.custom.yaml         the patches for default.yaml, nomime.yaml and the
//                         schemata (fonts, colours, layout, hotkeys, ...)
//   sinoime_match.yaml    suggestion mode, see MigrateSuggestionSettings()
//   user.yaml             Rime state: last build time, last used schema
//   build/                config compiled from the files above; its stale
//                         entries keep overriding the new defaults until the
//                         next deployment rebuilds it
//
// Kept -- the user's own data and the identity of this installation:
//   *.userdb/             typed words
//   *.userdb.txt, *.userdb.kct.snapshot, *_export.txt   dictionary snapshots
//   sync/                 backups, named after the installation id
//   installation.yaml     that installation id
//   anything else the user dropped in the folder by hand
const wchar_t* const kSettingsFiles[] = {
    L"sinoime_match.yaml",
    L"user.yaml",
};

const wchar_t kCompiledConfigDir[] = L"build";

// Preferences in the registry. RimeUserDir and Vi describe the installation
// itself rather than a preference, and the Updates subkey belongs to the
// updater -- clearing those would orphan the user's data or switch off the
// very update check that brought the new version in, so they stay.
const wchar_t* const kSettingsValues[] = {
    L"Language",
    L"ToggleImeOnOpenClose",
};

bool EndsWithNoCase(const std::wstring& text, const wchar_t* suffix) {
  size_t len = wcslen(suffix);
  return text.size() > len &&
         _wcsicmp(text.c_str() + text.size() - len, suffix) == 0;
}

bool IsSettingsFile(const std::wstring& name) {
  if (EndsWithNoCase(name, L".custom.yaml"))
    return true;
  for (const wchar_t* settings_file : kSettingsFiles) {
    if (_wcsicmp(name.c_str(), settings_file) == 0)
      return true;
  }
  return false;
}

std::wstring ReadSettingsVersion() {
  std::wstring version;
  if (RegGetStringValue(HKEY_CURRENT_USER, NOMIME_REG_KEY,
                        kSettingsVersionValue, version) != ERROR_SUCCESS)
    return std::wstring();
  return version;
}

void WriteSettingsVersion(const std::wstring& version) {
  RegSetKeyValueW(HKEY_CURRENT_USER, NOMIME_REG_KEY, kSettingsVersionValue,
                  REG_SZ, version.c_str(),
                  (DWORD)((version.size() + 1) * sizeof(wchar_t)));
}

void DeleteSettingsFiles() {
  const fs::path user_dir = NomIMEUserDataPath();
  std::error_code ec;
  // Collect first, delete after: removing entries from under an open directory
  // iterator is not something the iterator promises to survive.
  std::vector<fs::path> doomed;
  for (const fs::directory_entry& entry :
       fs::directory_iterator(user_dir, ec)) {
    if (entry.is_regular_file(ec) &&
        IsSettingsFile(entry.path().filename().wstring()))
      doomed.push_back(entry.path());
  }
  for (const fs::path& path : doomed) {
    fs::remove(path, ec);
    if (ec)
      DEBUG << L"NomIME: could not remove " << path.c_str();
  }
  fs::remove_all(user_dir / kCompiledConfigDir, ec);
  if (ec)
    DEBUG << L"NomIME: could not remove the compiled config folder";
}

void DeleteSettingsValues() {
  for (const wchar_t* value : kSettingsValues) {
    RegDeleteKeyValueW(HKEY_CURRENT_USER, NOMIME_REG_KEY, value);
  }
}

}  // namespace

bool ResetSettingsOnVersionChange() {
  const std::wstring current = u8tow(NOMIME_VERSION);
  if (current.empty())  // no version to tag the settings with; leave them be
    return false;
  if (ReadSettingsVersion() == current)
    return false;

  DeleteSettingsFiles();
  DeleteSettingsValues();
  WriteSettingsVersion(current);
  return true;
}
