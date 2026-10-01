#include "stdafx.h"
#include <string>
#include <vector>
#include <msctf.h>
#include <strsafe.h>
#include <StringAlgorithm.hpp>
#include <NomIMEConstants.h>
#include <NomIMEUtility.h>
#include "InstallOptionsDlg.h"

// {712579BA-DA20-4CBF-A387-98D7F463C968}
static const GUID c_clsidTextService = {
    0x712579ba,
    0xda20,
    0x4cbf,
    {0xa3, 0x87, 0x98, 0xd7, 0xf4, 0x63, 0xc9, 0x68}};

// {2EA97159-9D24-4072-941B-AC427B9B379E}
static const GUID c_guidProfile = {
    0x2ea97159,
    0x9d24,
    0x4072,
    {0x94, 0x1b, 0xac, 0x42, 0x7b, 0x9b, 0x37, 0x9e}};

// if in the future, option vi is extended, maybe a function to generate this
// info is required
#define PSZTITLE_HANS                                                     \
  L"0804:{712579BA-DA20-4CBF-A387-98D7F463C968}{2EA97159-9D24-4072-941B-" \
  L"AC427B9B379E}"
#define PSZTITLE_VI                                                       \
  L"042A:{712579BA-DA20-4CBF-A387-98D7F463C968}{2EA97159-9D24-4072-941B-" \
  L"AC427B9B379E}"
#define ILOT_UNINSTALL 0x00000001
typedef HRESULT(WINAPI* PTF_INSTALLLAYOUTORTIP)(LPCWSTR psz, DWORD dwFlags);

#define NOMIME_WER_KEY                            \
  L"SOFTWARE\\Microsoft\\Windows\\Windows Error " \
  L"Reporting\\LocalDumps\\NomIMEServer.exe"

BOOL copy_file(const std::wstring& src, const std::wstring& dest) {
  BOOL ret = CopyFile(src.c_str(), dest.c_str(), FALSE);
  if (!ret) {
    for (int i = 0; i < 10; ++i) {
      std::wstring old = dest + L".old." + std::to_wstring(i);
      if (MoveFileEx(dest.c_str(), old.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        MoveFileEx(old.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        break;
      }
    }
    ret = CopyFile(src.c_str(), dest.c_str(), FALSE);
  }
  return ret;
}

BOOL delete_file(const std::wstring& file) {
  BOOL ret = DeleteFile(file.c_str());
  if (!ret) {
    for (int i = 0; i < 10; ++i) {
      std::wstring old = file + L".old." + std::to_wstring(i);
      if (MoveFileEx(file.c_str(), old.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        MoveFileEx(old.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        return TRUE;
      }
    }
  }
  return ret;
}

typedef BOOL(WINAPI* PISWOW64P2)(HANDLE, USHORT*, USHORT*);
BOOL is_arm64_machine() {
  PISWOW64P2 fnIsWow64Process2 = (PISWOW64P2)GetProcAddress(
      GetModuleHandle(_T("kernel32.dll")), "IsWow64Process2");

  if (fnIsWow64Process2 == NULL) {
    return FALSE;
  }

  USHORT processMachine;
  USHORT nativeMachine;

  if (!fnIsWow64Process2(GetCurrentProcess(), &processMachine,
                         &nativeMachine)) {
    return FALSE;
  }
  return nativeMachine == IMAGE_FILE_MACHINE_ARM64;
}

typedef HRESULT(WINAPI* PISWOWGMS)(USHORT, BOOL*);
typedef UINT(WINAPI* PGSW64DIR2)(LPWSTR, UINT, WORD);
INT get_wow_arm32_system_dir(LPWSTR lpBuffer, UINT uSize) {
  PISWOWGMS fnIsWow64GuestMachineSupported = (PISWOWGMS)GetProcAddress(
      GetModuleHandle(_T("kernel32.dll")), "IsWow64GuestMachineSupported");
  PGSW64DIR2 fnGetSystemWow64Directory2W = (PGSW64DIR2)GetProcAddress(
      GetModuleHandle(_T("kernelbase.dll")), "GetSystemWow64Directory2W");

  if (fnIsWow64GuestMachineSupported == NULL ||
      fnGetSystemWow64Directory2W == NULL) {
    return 0;
  }

  BOOL supported;
  if (fnIsWow64GuestMachineSupported(IMAGE_FILE_MACHINE_ARMNT, &supported) !=
      S_OK) {
    return 0;
  }

  if (!supported) {
    return 0;
  }

  return fnGetSystemWow64Directory2W(lpBuffer, uSize, IMAGE_FILE_MACHINE_ARMNT);
}

typedef int (*ime_register_func)(const std::wstring& ime_path,
                                 bool register_ime,
                                 bool is_wow64,
                                 bool is_wowarm,
                                 bool vi,
                                 bool silent);

int install_ime_file(std::wstring& srcPath,
                     const std::wstring& ext,
                     bool vi,
                     bool silent,
                     ime_register_func func) {
  WCHAR path[MAX_PATH];
  GetModuleFileNameW(GetModuleHandle(NULL), path, _countof(path));

  std::wstring srcFileName = L"nomime";

  srcFileName += ext;
  WCHAR drive[_MAX_DRIVE];
  WCHAR dir[_MAX_DIR];
  _wsplitpath_s(path, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL,
                0);
  srcPath = std::wstring(drive) + dir + srcFileName;

  GetSystemDirectoryW(path, _countof(path));
  std::wstring destPath = std::wstring(path) + L"\\nomime" + ext;

  int retval = 0;
  // 复制 .dll/.ime 到系统目录
  if (!copy_file(srcPath, destPath)) {
    MSG_NOT_SILENT_ID_CAP(silent, destPath.c_str(), IDS_STR_INSTALL_FAILED,
                          MB_ICONERROR | MB_OK);
    return 1;
  }
  retval += func(destPath, true, false, false, vi, silent);
  if (is_wow64()) {
    PVOID OldValue = NULL;
    // PW64DW64FR fnWow64DisableWow64FsRedirection =
    // (PW64DW64FR)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),
    // "Wow64DisableWow64FsRedirection"); PW64RW64FR
    // fnWow64RevertWow64FsRedirection =
    // (PW64RW64FR)GetProcAddress(GetModuleHandle(_T("kernel32.dll")),
    // "Wow64RevertWow64FsRedirection");
    if (Wow64DisableWow64FsRedirection(&OldValue) == FALSE) {
      MSG_NOT_SILENT_BY_IDS(silent, IDS_STR_ERRCANCELFSREDIRECT,
                            IDS_STR_INSTALL_FAILED, MB_ICONERROR | MB_OK);
      return 1;
    }

    if (is_arm64_machine()) {
      WCHAR sysarm32[MAX_PATH];
      if (get_wow_arm32_system_dir(sysarm32, _countof(sysarm32)) > 0) {
        // Install the ARM32 version if ARM32 WOW is supported （lower than
        // Windows 11 24H2).
        std::wstring srcPathARM32 = srcPath;
        ireplace_last(srcPathARM32, ext, L"ARM" + ext);

        std::wstring destPathARM32 = std::wstring(sysarm32) + L"\\nomime" + ext;
        if (!copy_file(srcPathARM32, destPathARM32)) {
          MSG_NOT_SILENT_ID_CAP(silent, destPathARM32.c_str(),
                                IDS_STR_INSTALL_FAILED, MB_ICONERROR | MB_OK);
          return 1;
        }
        retval += func(destPathARM32, true, true, true, vi, silent);
      }

      // Then install the ARM64 (and x64) version.
      // On ARM64 nomime.dll(ime) is an ARM64X redirection DLL (nomimeARM64X).
      // When loaded, it will be redirected to nomimeARM64.dll(ime) on ARM64
      // processes, and nomimex64.dll(ime) on x64 processes. So we need a total
      // of three files.

      std::wstring srcPathX64 = srcPath;
      std::wstring destPathX64 = destPath;
      ireplace_last(srcPathX64, ext, L"x64" + ext);
      ireplace_last(destPathX64, ext, L"x64" + ext);
      if (!copy_file(srcPathX64, destPathX64)) {
        MSG_NOT_SILENT_ID_CAP(silent, destPathX64.c_str(),
                              IDS_STR_INSTALL_FAILED, MB_ICONERROR | MB_OK);
        return 1;
      }

      std::wstring srcPathARM64 = srcPath;
      std::wstring destPathARM64 = destPath;
      ireplace_last(srcPathARM64, ext, L"ARM64" + ext);
      ireplace_last(destPathARM64, ext, L"ARM64" + ext);
      if (!copy_file(srcPathARM64, destPathARM64)) {
        MSG_NOT_SILENT_ID_CAP(silent, destPathARM64.c_str(),
                              IDS_STR_INSTALL_FAILED, MB_ICONERROR | MB_OK);
        return 1;
      }

      // Since nomimeARM64X is just a redirector we don't have separate
      // HANS and VI variants.
      srcPath = std::wstring(drive) + dir + L"nomimeARM64X" + ext;
    } else {
      ireplace_last(srcPath, ext, L"x64" + ext);
    }

    if (!copy_file(srcPath, destPath)) {
      MSG_NOT_SILENT_ID_CAP(silent, destPath.c_str(), IDS_STR_INSTALL_FAILED,
                            MB_ICONERROR | MB_OK);
      return 1;
    }
    retval += func(destPath, true, true, false, vi, silent);
    if (Wow64RevertWow64FsRedirection(OldValue) == FALSE) {
      MSG_NOT_SILENT_BY_IDS(silent, IDS_STR_ERRRECOVERFSREDIRECT,
                            IDS_STR_INSTALL_FAILED, MB_ICONERROR | MB_OK);
      return 1;
    }
  }
  return retval;
}

int uninstall_ime_file(const std::wstring& ext,
                       bool silent,
                       ime_register_func func) {
  int retval = 0;
  WCHAR path[MAX_PATH];
  GetSystemDirectoryW(path, _countof(path));
  std::wstring imePath(path);
  imePath += L"\\nomime" + ext;
  retval += func(imePath, false, false, false, false, silent);
  delete_file(imePath);
  if (is_wow64()) {
    retval += func(imePath, false, true, false, false, silent);
    PVOID OldValue = NULL;
    if (Wow64DisableWow64FsRedirection(&OldValue) == FALSE) {
      MSG_NOT_SILENT_BY_IDS(silent, IDS_STR_ERRCANCELFSREDIRECT,
                            IDS_STR_UNINSTALL_FAILED, MB_ICONERROR | MB_OK);
      return 1;
    }

    if (is_arm64_machine()) {
      WCHAR sysarm32[MAX_PATH];
      if (get_wow_arm32_system_dir(sysarm32, _countof(sysarm32)) > 0) {
        std::wstring imePathARM32 = std::wstring(sysarm32) + L"\\nomime" + ext;
        retval += func(imePathARM32, false, true, true, false, silent);
        delete_file(imePathARM32);
      }

      std::wstring imePathX64 = imePath;
      ireplace_last(imePathX64, ext, L"x64" + ext);
      delete_file(imePathX64);

      std::wstring imePathARM64 = imePath;
      ireplace_last(imePathARM64, ext, L"ARM64" + ext);
      delete_file(imePathARM64);
    }

    delete_file(imePath);
    if (Wow64RevertWow64FsRedirection(OldValue) == FALSE) {
      MSG_NOT_SILENT_BY_IDS(silent, IDS_STR_ERRRECOVERFSREDIRECT,
                            IDS_STR_UNINSTALL_FAILED, MB_ICONERROR | MB_OK);
      return 1;
    }
  }
  return retval;
}

// 注册IME输入法
// `register_ime` (IMM/.ime) support removed — TSF-only build

void enable_profile(BOOL fEnable, bool vi) {
  HRESULT hr;
  ITfInputProcessorProfiles* pProfiles = NULL;

  hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL,
                        CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfiles,
                        (LPVOID*)&pProfiles);

  if (SUCCEEDED(hr)) {
    LANGID lang_id = vi ? 0x042A : 0x0804;
    if (fEnable) {
      pProfiles->EnableLanguageProfile(c_clsidTextService, lang_id,
                                       c_guidProfile, fEnable);
      pProfiles->EnableLanguageProfileByDefault(c_clsidTextService, lang_id,
                                                c_guidProfile, fEnable);
    } else {
      pProfiles->RemoveLanguageProfile(c_clsidTextService, lang_id,
                                       c_guidProfile);
    }

    pProfiles->Release();
  }
}

// 注册TSF输入法
int register_text_service(const std::wstring& tsf_path,
                          bool register_ime,
                          bool is_wow64,
                          bool is_wowarm32,
                          bool vi,
                          bool silent) {
  using RegisterServerFunction = HRESULT(STDAPICALLTYPE*)();

  if (!register_ime)
    enable_profile(FALSE, vi);

  std::wstring params = L" \"" + tsf_path + L"\"";
  if (!register_ime) {
    params = L" /u " + params;  // unregister
  }
  // if (silent)  // always silent
  { params = L" /s " + params; }

  if (!SetEnvironmentVariable(L"TEXTSERVICE_PROFILE", vi ? L"vi" : L"hans")) {
    throw std::runtime_error("SetEnvironmentVariable failed");
  }

  std::wstring app = L"regsvr32.exe";
  if (is_wowarm32) {
    WCHAR sysarm32[MAX_PATH];
    get_wow_arm32_system_dir(sysarm32, _countof(sysarm32));

    app = std::wstring(sysarm32) + L"\\" + app;
  }

  SHELLEXECUTEINFOW shExInfo = {0};
  shExInfo.cbSize = sizeof(shExInfo);
  shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  shExInfo.hwnd = 0;
  shExInfo.lpVerb = L"open";               // Operation to perform
  shExInfo.lpFile = app.c_str();           // Application to start
  shExInfo.lpParameters = params.c_str();  // Additional parameters
  shExInfo.lpDirectory = 0;
  shExInfo.nShow = SW_SHOW;
  shExInfo.hInstApp = 0;
  if (ShellExecuteExW(&shExInfo)) {
    WaitForSingleObject(shExInfo.hProcess, INFINITE);
    CloseHandle(shExInfo.hProcess);
  } else {
    WCHAR msg[100];
    CString str;
    str.LoadStringW(IDS_STR_ERRREGTSF);
    StringCchPrintfW(msg, _countof(msg), str, params.c_str());
    MSG_NOT_SILENT_ID_CAP(silent, msg, IDS_STR_INORUN_FAILED,
                          MB_ICONERROR | MB_OK);
    return 1;
  }

  if (register_ime)
    enable_profile(TRUE, vi);

  return 0;
}

// whether the user has added the Vietnamese keyboard layout (042A:0000042A)
// on their own, i.e. it is listed in HKCU\Keyboard Layout\Preload
static bool user_has_vi_keyboard() {
  const WCHAR KLID_VI[] = L"0000042a";
  HKEY hPreload;
  if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Keyboard Layout\\Preload", 0, KEY_READ,
                    &hPreload) != ERROR_SUCCESS)
    return false;
  HKEY hSubst = NULL;
  RegOpenKeyExW(HKEY_CURRENT_USER, L"Keyboard Layout\\Substitutes", 0, KEY_READ,
                &hSubst);

  bool found = false;
  for (DWORD i = 0; !found; ++i) {
    WCHAR name[16];
    WCHAR klid[16] = {0};
    DWORD name_len = _countof(name);
    DWORD type = 0;
    DWORD size = sizeof(klid) - sizeof(WCHAR);
    LSTATUS ret = RegEnumValueW(hPreload, i, name, &name_len, NULL, &type,
                                (LPBYTE)klid, &size);
    if (ret == ERROR_MORE_DATA)
      continue;
    if (ret != ERROR_SUCCESS)
      break;
    if (type != REG_SZ)
      continue;
    // language part of the preload entry must be Vietnamese
    if (wcslen(klid) != 8 || _wcsicmp(klid + 4, L"042a") != 0)
      continue;
    // the entry may be redirected to another layout, e.g. the hidden US
    // layout backing the TIP (0000042a -> 00000409)
    WCHAR subst[16] = {0};
    size = sizeof(subst) - sizeof(WCHAR);
    if (hSubst &&
        RegQueryValueExW(hSubst, klid, NULL, &type, (LPBYTE)subst, &size) ==
            ERROR_SUCCESS &&
        type == REG_SZ)
      found = (_wcsicmp(subst, KLID_VI) == 0);
    else
      found = (_wcsicmp(klid, KLID_VI) == 0);
  }

  if (hSubst)
    RegCloseKey(hSubst);
  RegCloseKey(hPreload);
  return found;
}

// Registering the TIP under 0x042A while Vietnamese is not in the user's
// language list makes Windows load the stock Vietnamese keyboard into the
// current session, which then shows up in the input switcher as an extra
// "VIE Vietnamese" entry. Unload it unless the user added it themselves.
static void unload_stray_vi_keyboard() {
  const UINT_PTR HKL_VI = 0x042A042A;
  if (user_has_vi_keyboard())
    return;
  int count = GetKeyboardLayoutList(0, NULL);
  if (count <= 0)
    return;
  std::vector<HKL> layouts(count);
  count = GetKeyboardLayoutList(count, layouts.data());
  for (int i = 0; i < count; ++i) {
    if (((UINT_PTR)layouts[i] & 0xFFFFFFFF) == HKL_VI) {
      UnloadKeyboardLayout(layouts[i]);
      break;
    }
  }
}

int install(bool vi, bool silent) {
  std::wstring ime_src_path;
  int retval = 0;

  retval += install_ime_file(ime_src_path, L".dll", vi, silent,
                             &register_text_service);

  // 写注册表
  WCHAR drive[_MAX_DRIVE];
  WCHAR dir[_MAX_DIR];
  _wsplitpath_s(ime_src_path.c_str(), drive, _countof(drive), dir,
                _countof(dir), NULL, 0, NULL, 0);
  std::wstring rootDir = std::wstring(drive) + dir;
  rootDir.pop_back();
  auto ret = SetRegKeyValue(HKEY_LOCAL_MACHINE, NOMIME_REG_KEY, L"NomIMERoot",
                            rootDir.c_str(), REG_SZ);
  if (FAILED(HRESULT_FROM_WIN32(ret))) {
    MSG_NOT_SILENT_BY_IDS(silent, IDS_STR_ERRWRITENOMIMEROOT,
                          IDS_STR_INSTALL_FAILED, MB_ICONERROR | MB_OK);
    return 1;
  }

  const std::wstring executable = L"NomIMEServer.exe";
  ret = SetRegKeyValue(HKEY_LOCAL_MACHINE, NOMIME_REG_KEY, L"ServerExecutable",
                       executable.c_str(), REG_SZ);
  if (FAILED(HRESULT_FROM_WIN32(ret))) {
    MSG_NOT_SILENT_BY_IDS(silent, IDS_STR_ERRREGIMEWRITESVREXE,
                          IDS_STR_INSTALL_FAILED, MB_ICONERROR | MB_OK);
    return 1;
  }

  // InstallLayoutOrTip
  // https://learn.microsoft.com/zh-cn/windows/win32/tsf/installlayoutortip
  // example in ref page not right with "*PTF_ INSTALLLAYOUTORTIP"
  // space inside should be removed
  HMODULE hInputDLL = LoadLibrary(TEXT("input.dll"));
  if (hInputDLL) {
    PTF_INSTALLLAYOUTORTIP pfnInstallLayoutOrTip;
    pfnInstallLayoutOrTip =
        (PTF_INSTALLLAYOUTORTIP)GetProcAddress(hInputDLL, "InstallLayoutOrTip");
    if (pfnInstallLayoutOrTip) {
      if (vi)
        (*pfnInstallLayoutOrTip)(PSZTITLE_VI, 0);
      else
        (*pfnInstallLayoutOrTip)(PSZTITLE_HANS, 0);
    }
    FreeLibrary(hInputDLL);
  }
  if (vi)
    unload_stray_vi_keyboard();

  // https://learn.microsoft.com/zh-cn/windows/win32/wer/collecting-user-mode-dumps
  const std::wstring dmpPathW = NomIMELogPath().wstring();
  // DumpFolder
  SetRegKeyValue(HKEY_LOCAL_MACHINE, NOMIME_WER_KEY, L"DumpFolder",
                 dmpPathW.c_str(), REG_SZ, true);
  // dump type 0
  SetRegKeyValue(HKEY_LOCAL_MACHINE, NOMIME_WER_KEY, L"DumpType", 0, REG_DWORD,
                 true);
  // CustomDumpFlags, MiniDumpNormal
  SetRegKeyValue(HKEY_LOCAL_MACHINE, NOMIME_WER_KEY, L"CustomDumpFlags", 0,
                 REG_DWORD, true);
  // maximium dump count 10
  SetRegKeyValue(HKEY_LOCAL_MACHINE, NOMIME_WER_KEY, L"DumpCount", 10,
                 REG_DWORD, true);

  if (retval)
    return 1;

  MSG_NOT_SILENT_BY_IDS(silent, IDS_STR_INSTALL_SUCCESS_INFO,
                        IDS_STR_INSTALL_SUCCESS_CAP,
                        MB_ICONINFORMATION | MB_OK);
  return 0;
}

int uninstall(bool silent) {
  // 注销输入法
  int retval = 0;

  const WCHAR KEY[] = L"Software\\SinoNom\\NomIME";
  HKEY hKey;
  LSTATUS ret = RegOpenKey(HKEY_CURRENT_USER, KEY, &hKey);
  if (ret == ERROR_SUCCESS) {
    DWORD type = 0;
    DWORD data = 0;
    DWORD len = sizeof(data);
    ret = RegQueryValueEx(hKey, L"Vi", NULL, &type, (LPBYTE)&data, &len);
    if (ret == ERROR_SUCCESS && type == REG_DWORD) {
      HMODULE hInputDLL = LoadLibrary(TEXT("input.dll"));
      if (hInputDLL) {
        PTF_INSTALLLAYOUTORTIP pfnInstallLayoutOrTip;
        pfnInstallLayoutOrTip = (PTF_INSTALLLAYOUTORTIP)GetProcAddress(
            hInputDLL, "InstallLayoutOrTip");
        if (pfnInstallLayoutOrTip) {
          if (data != 0)
            (*pfnInstallLayoutOrTip)(PSZTITLE_VI, ILOT_UNINSTALL);
          else
            (*pfnInstallLayoutOrTip)(PSZTITLE_HANS, ILOT_UNINSTALL);
        }
        FreeLibrary(hInputDLL);
      }
    }
    RegCloseKey(hKey);
  }

  // IMM/.ime support removed; only uninstall TSF/.dll
  retval += uninstall_ime_file(L".dll", silent, &register_text_service);
  unload_stray_vi_keyboard();

  // 清除注册信息
  RegDeleteKey(HKEY_LOCAL_MACHINE, NOMIME_REG_KEY);
  RegDeleteKey(HKEY_LOCAL_MACHINE, RIME_REG_KEY);

  // delete WER register,
  // "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\Windows Error
  // Reporting\\LocalDumps\\NomIMEServer.exe" no WOW64 redirect

  auto flag_wow64 = is_wow64() ? KEY_WOW64_64KEY : 0;
  RegDeleteKeyEx(HKEY_LOCAL_MACHINE, NOMIME_WER_KEY, flag_wow64, 0);
  if (retval)
    return 1;

  MSG_NOT_SILENT_BY_IDS(silent, IDS_STR_UNINSTALL_SUCCESS_INFO,
                        IDS_STR_UNINSTALL_SUCCESS_CAP,
                        MB_ICONINFORMATION | MB_OK);
  return 0;
}

bool has_installed() {
  WCHAR path[MAX_PATH];
  GetSystemDirectory(path, _countof(path));
  std::wstring sysPath(path);
  DWORD attr = GetFileAttributesW((sysPath + L"\\nomime.dll").c_str());
  return (attr != INVALID_FILE_ATTRIBUTES &&
          !(attr & FILE_ATTRIBUTE_DIRECTORY));
}
