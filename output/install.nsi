; nomime installation script
!include FileFunc.nsh
!include LogicLib.nsh
!include MUI2.nsh
!include x64.nsh
!include winVer.nsh

Unicode true

;--------------------------------
; General

!ifndef NOMIME_VERSION
!define NOMIME_VERSION 1.0.7
!endif

!ifndef NOMIME_BUILD
!define NOMIME_BUILD 0
!endif

!define NOMIME_ROOT $INSTDIR\NomIME-${NOMIME_VERSION}
!define REG_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\NomIME"

; The name of the installer
Name "NomIME ${NOMIME_VERSION}"

; The file to write
OutFile "archives\NomIME-${PRODUCT_VERSION}-installer.exe"

VIProductVersion "${NOMIME_VERSION}.${NOMIME_BUILD}"
VIAddVersionKey /LANG=1066 "ProductName" "NomIME"
VIAddVersionKey /LANG=1066 "Comments" "Powered by Viễn Chi | Bộ gõ Hán Nôm - Phương Viên"
VIAddVersionKey /LANG=1066 "CompanyName" "Phương Viên"
VIAddVersionKey /LANG=1066 "LegalCopyright" "Copyleft Phương Viên"
VIAddVersionKey /LANG=1066 "FileDescription" "Bộ gõ Hán Nôm - Phương Viên"
VIAddVersionKey /LANG=1066 "FileVersion" "${NOMIME_VERSION}"

!define MUI_ICON ..\resource\nomime.ico
SetCompressor /SOLID lzma


; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

!insertmacro MUI_RESERVEFILE_LANGDLL

!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------

; Languages

!insertmacro MUI_LANGUAGE "Vietnamese"
LangString DISPLAYNAME ${LANG_VIETNAMESE} "Phương Viên - NomIME"
LangString LNKFORMANUAL ${LANG_VIETNAMESE} "Hướng dẫn sử dụng NomIME"
LangString LNKFORSETTING ${LANG_VIETNAMESE} "Cài đặt Phương Viên - NomIME"
LangString LNKFORDICT ${LANG_VIETNAMESE} "Quản lý từ điển Phương Viên - NomIME"
LangString LNKFORSYNC ${LANG_VIETNAMESE} "Đồng bộ hồ sơ người dùng Phương Viên - NomIME"
LangString LNKFORDEPLOY ${LANG_VIETNAMESE} "Triển khai lại Phương Viên - NomIME"
LangString LNKFORSERVER ${LANG_VIETNAMESE} "Máy chủ Phương Viên - NomIME"
LangString LNKFORUSERFOLDER ${LANG_VIETNAMESE} "Thư mục người dùng Phương Viên - NomIME"
LangString LNKFORAPPFOLDER ${LANG_VIETNAMESE} "Thư mục ứng dụng Phương Viên - NomIME"
LangString LNKFORUPDATER ${LANG_VIETNAMESE} "Kiểm tra cập nhật Phương Viên - NomIME"
LangString LNKFORSETUP ${LANG_VIETNAMESE} "Tùy chọn cài đặt Phương Viên - NomIME"
LangString LNKFORUNINSTALL ${LANG_VIETNAMESE} "Gỡ cài đặt Phương Viên - NomIME"
LangString CONFIRMATION ${LANG_VIETNAMESE} "Trước khi cài đặt, vui lòng gỡ bỏ phiên bản Phương Viên - NomIME cũ.$\n$\nNhấn 'OK' để gỡ bỏ phiên bản cũ, hoặc 'Cancel' để hủy cài đặt."
LangString SYSTEMVERSIONNOTOK ${LANG_VIETNAMESE} "Hệ điều hành của bạn không được hỗ trợ. Yêu cầu tối thiểu: Windows 8.1!"
LangString AUTOCHKUPDATE ${LANG_VIETNAMESE} "Tự động kiểm tra cập nhật phiên bản mới?"

!insertmacro MUI_LANGUAGE "TradChinese"
LangString DISPLAYNAME ${LANG_TRADCHINESE} "NomIME輸入法"
LangString LNKFORMANUAL ${LANG_TRADCHINESE} "(Phương Viên - NomIME) 說明書"
LangString LNKFORSETTING ${LANG_TRADCHINESE} "(Phương Viên - NomIME) 輸入法設定"
LangString LNKFORDICT ${LANG_TRADCHINESE} "(Phương Viên - NomIME) 用戶詞典管理"
LangString LNKFORSYNC ${LANG_TRADCHINESE} "(Phương Viên - NomIME) 用戶資料同步"
LangString LNKFORDEPLOY ${LANG_TRADCHINESE} "(Phương Viên - NomIME) 重新部署"
LangString LNKFORSERVER ${LANG_TRADCHINESE} "NomIME算法服務"
LangString LNKFORUSERFOLDER ${LANG_TRADCHINESE} "(Phương Viên - NomIME) 用戶文件夾"
LangString LNKFORAPPFOLDER ${LANG_TRADCHINESE} "(Phương Viên - NomIME) 程序文件夾"
LangString LNKFORUPDATER ${LANG_TRADCHINESE} "(Phương Viên - NomIME) 檢查新版本"
LangString LNKFORSETUP ${LANG_TRADCHINESE} "(Phương Viên - NomIME) 安裝選項"
LangString LNKFORUNINSTALL ${LANG_TRADCHINESE} "卸載NomIME"
LangString CONFIRMATION ${LANG_TRADCHINESE} "安裝前，請先卸載舊版本的NomIME。$\n$\n按下「確定」移除舊版本，按下「取消」放棄本次安裝。"
LangString SYSTEMVERSIONNOTOK ${LANG_TRADCHINESE} "您的系统不被支持，最低系統要求:Windows 8.1!"
LangString AUTOCHKUPDATE ${LANG_TRADCHINESE} "自動檢查版本更新？"

!insertmacro MUI_LANGUAGE "SimpChinese"
LangString DISPLAYNAME ${LANG_SIMPCHINESE} "NomIME输入法"
LangString LNKFORMANUAL ${LANG_SIMPCHINESE} "(Phương Viên - NomIME) 说明书"
LangString LNKFORSETTING ${LANG_SIMPCHINESE} "(Phương Viên - NomIME) 输入法设定"
LangString LNKFORDICT ${LANG_SIMPCHINESE} "(Phương Viên - NomIME) 用户词典管理"
LangString LNKFORSYNC ${LANG_SIMPCHINESE} "(Phương Viên - NomIME) 用户资料同步"
LangString LNKFORDEPLOY ${LANG_SIMPCHINESE} "(Phương Viên - NomIME) 重新部署"
LangString LNKFORSERVER ${LANG_SIMPCHINESE} "NomIME算法服务"
LangString LNKFORUSERFOLDER ${LANG_SIMPCHINESE} "(Phương Viên - NomIME) 用户文件夹"
LangString LNKFORAPPFOLDER ${LANG_SIMPCHINESE} "(Phương Viên - NomIME) 程序文件夹"
LangString LNKFORUPDATER ${LANG_SIMPCHINESE} "(Phương Viên - NomIME) 检查新版本"
LangString LNKFORSETUP ${LANG_SIMPCHINESE} "(Phương Viên - NomIME) 安装选项"
LangString LNKFORUNINSTALL ${LANG_SIMPCHINESE} "卸载NomIME"
LangString CONFIRMATION ${LANG_SIMPCHINESE} '安装前，请先卸载旧版本的NomIME。$\n$\n点击 "确定" 移除旧版本，或点击 "取消" 放弃本次安装。'
LangString SYSTEMVERSIONNOTOK ${LANG_SIMPCHINESE} "您的系統不被支持，最低系统要求:Windows 8.1!"
LangString AUTOCHKUPDATE ${LANG_SIMPCHINESE} "自动检查版本更新？"

!insertmacro MUI_LANGUAGE "English"
LangString DISPLAYNAME ${LANG_ENGLISH} "NomIME"
LangString LNKFORMANUAL ${LANG_ENGLISH} "NomIME Manual"
LangString LNKFORSETTING ${LANG_ENGLISH} "NomIME Settings"
LangString LNKFORDICT ${LANG_ENGLISH} "NomIME Dictionary Manager"
LangString LNKFORSYNC ${LANG_ENGLISH} "NomIME Sync User Profile"
LangString LNKFORDEPLOY ${LANG_ENGLISH} "NomIME Deploy"
LangString LNKFORSERVER ${LANG_ENGLISH} "NomIME Server"
LangString LNKFORUSERFOLDER ${LANG_ENGLISH} "NomIME User Folder"
LangString LNKFORAPPFOLDER ${LANG_ENGLISH} "NomIME App Folder"
LangString LNKFORUPDATER ${LANG_ENGLISH} "NomIME Check for Updates"
LangString LNKFORSETUP ${LANG_ENGLISH} "NomIME Installation Preference"
LangString LNKFORUNINSTALL ${LANG_ENGLISH} "Uninstall NomIME"
LangString CONFIRMATION ${LANG_ENGLISH} "Before installation, please uninstall the old version of NomIME.$\n$\nPress 'OK' to remove the old version, or 'Cancel' to abort installation."
LangString SYSTEMVERSIONNOTOK ${LANG_ENGLISH} "Your system not supported, minimium system required: Windows 8.1!"
LangString AUTOCHKUPDATE ${LANG_ENGLISH} "Automatically check for updates?"

;--------------------------------

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
  ; if not version >= 8.1, quit and MessageBox(if not silent)
  ${IfNot} ${AtLeastWin8.1}
    IfSilent toquit
    MessageBox MB_OK '$(SYSTEMVERSIONNOTOK)'
toquit:
    Quit
  ${EndIf}

  ReadRegStr $R0 HKLM "Software\NomIME\NomIME" "InstallDir"
  StrCmp $R0 "" 0 skip
  ; The default installation directory
  ; install x64 build for NativeARM64_WINDOWS11 and NativeAMD64_WINDOWS11
  ${If} ${AtLeastWin11} ; Windows 11 and above
    ${If} ${IsNativeARM64}
      StrCpy $INSTDIR "$PROGRAMFILES64\NomIME"
    ${ElseIf} ${IsNativeAMD64}
      StrCpy $INSTDIR "$PROGRAMFILES64\NomIME"
    ${Else}
      StrCpy $INSTDIR "$PROGRAMFILES\NomIME"
    ${Endif}
  ; install x64 build for NativeAMD64_BELLOW_WINDOWS11
  ${Else} ; Windows 10 or bellow
    ${If} ${IsNativeAMD64}
      StrCpy $INSTDIR "$PROGRAMFILES64\NomIME"
    ${Else}
      StrCpy $INSTDIR "$PROGRAMFILES\NomIME"
    ${Endif}
  ${Endif}
skip:
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\NomIME" \
  "UninstallString"
  StrCmp $R0 "" done

  StrCpy $0 "Upgrade"
  IfSilent uninst 0
  MessageBox MB_OKCANCEL|MB_ICONINFORMATION "$(CONFIRMATION)" IDOK uninst
  Abort

uninst:
  ; Backup data directory from previous installation, user files may exist
  ReadRegStr $R1 HKLM SOFTWARE\NomIME\NomIME "NomIMERoot"
  StrCmp $R1 "" call_uninstaller
  IfFileExists $R1\data\*.* 0 call_uninstaller
  CreateDirectory $TEMP\nomime-backup
  CopyFiles $R1\data\*.* $TEMP\nomime-backup

call_uninstaller:
  ExecWait '"$R1\NomIMEServer.exe" /quit'
  ExecWait '"$R1\NomIMESetup.exe" /u'
  ; Remove registry keys
  DeleteRegKey HKLM SOFTWARE\NomIME
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NomIME"
  ; don't redirect on 64 bit system for auto run setting
  ${If} ${IsNativeARM64}
    SetRegView 64
  ${ElseIf} ${IsNativeAMD64}
    SetRegView 64
  ${Endif}
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "NomIMEServer"
  ; recover back to 32bit view
  SetRegView 32
  ; Remove files and uninstaller
  Delete  "$R1\data\opencc\*.*"
  Delete  "$R1\data\preview\*.*"
  Delete  "$R1\data\*.*"
  Delete  "$R1\*.*"
  RMDir   "$R1\data\opencc"
  RMDir   "$R1\data\preview"
  RMDir   "$R1\data"
  RMDir   "$R1"
  SetShellVarContext all
  Delete  "$SMPROGRAMS\$(DISPLAYNAME)\*.*"
  RMDir  "$SMPROGRAMS\$(DISPLAYNAME)"
  ; Prompt reboot
  SetRebootFlag true
  Sleep 800

done:
FunctionEnd

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\NomIME\NomIME" "InstallDir"

; The stuff to install
Section "NomIME"

  SectionIn RO

  ; Write the new installation path into the registry
  ; redirect on 64 bit system
  ; HKLM SOFTWARE\WOW6432Node\NomIME\NomIME "InstallDir" "$INSTDIR"
  WriteRegStr HKLM SOFTWARE\NomIME\NomIME "InstallDir" "$INSTDIR"

  ; Reset INSTDIR for the new version
  StrCpy $INSTDIR "${NOMIME_ROOT}"

  IfFileExists "$INSTDIR\NomIMEServer.exe" 0 +2
  ExecWait '"$INSTDIR\NomIMEServer.exe" /quit'

  SetOverwrite try
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  IfFileExists $TEMP\nomime-backup\*.* 0 program_files
  CreateDirectory $INSTDIR\data
  CopyFiles $TEMP\nomime-backup\*.* $INSTDIR\data
  RMDir /r $TEMP\nomime-backup

program_files:
  File "LICENSE.txt"
  File "README.txt"
  File "7-zip-license.txt"
  File "7z.dll"
  File "7z.exe"
  File "COPYING-curl.txt"
  File "curl.exe"
  File "curl-ca-bundle.crt"
  File "rime-install.bat"
  File "rime-install-config.bat"
  File "start_service.bat"
  File "stop_service.bat"
  File "nomime.dll"
  ${If} ${RunningX64}
    File "nomimex64.dll"
  ${EndIf}
  ${If} ${IsNativeARM64}
    File /nonfatal "nomimeARM.dll"
    File /nonfatal "nomimeARM64.dll"
    File /nonfatal "nomimeARM64X.dll"
  ${EndIf}
  ; install x64 build for NativeARM64_WINDOWS11 and NativeAMD64_WINDOWS11
  ${If} ${AtLeastWin11} ; Windows 11 and above
    ${If} ${IsNativeARM64}
      File "NomIMEDeployer.exe"
      File "NomIMEServer.exe"
      File "rime.dll"
      File "WinSparkle.dll"
    ${ElseIf} ${IsNativeAMD64}
      File "NomIMEDeployer.exe"
      File "NomIMEServer.exe"
      File "rime.dll"
      File "WinSparkle.dll"
    ${Else}
      File "Win32\NomIMEDeployer.exe"
      File "Win32\NomIMEServer.exe"
      File "Win32\NomIME.dll"
      File "Win32\WinSparkle.dll"
    ${Endif}
  ; install x64 build for NativeAMD64_BELLOW_WINDOWS11
  ${Else} ; Windows 10 or bellow
    ${If} ${IsNativeAMD64}
      File "NomIMEDeployer.exe"
      File "NomIMEServer.exe"
      File "rime.dll"
      File "WinSparkle.dll"
    ${Else}
      File "Win32\NomIMEDeployer.exe"
      File "Win32\NomIMEServer.exe"
      File "Win32\NomIME.dll"
      File "Win32\WinSparkle.dll"
    ${Endif}
  ${Endif}

  File "NomIMESetup.exe"
  ; shared data files
  SetOutPath $INSTDIR\data
  File "data\*.yaml"
  File /nonfatal "data\*.txt"
  File /nonfatal "data\*.gram"
  ; opencc data files
  SetOutPath $INSTDIR\data\opencc
  File "data\opencc\*.json"
  File "data\opencc\*.ocd*"
  ; images
  SetOutPath $INSTDIR\data\preview
  File "data\preview\*.png"

  SetOutPath $INSTDIR

  ; test /T flag for zh_TW locale
  StrCpy $R2 "/i"
  ${GetParameters} $R0
  ClearErrors
  ${GetOptions} $R0 "/S" $R1
  IfErrors +2 0
  StrCpy $R2 "/s"
  ${GetOptions} $R0 "/T" $R1
  IfErrors +2 0
  StrCpy $R2 "/t"

  ExecWait '"$INSTDIR\NomIMESetup.exe" $R2'

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "${REG_UNINST_KEY}" "DisplayName" "$(DISPLAYNAME)"
  WriteRegStr HKLM "${REG_UNINST_KEY}" "DisplayIcon" '"$INSTDIR\NomIMEServer.exe"'
  WriteRegStr HKLM "${REG_UNINST_KEY}" "DisplayVersion" "${NOMIME_VERSION}.${NOMIME_BUILD}"
  WriteRegStr HKLM "${REG_UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "${REG_UNINST_KEY}" "Publisher" "Phương Viên"
  WriteRegStr HKLM "${REG_UNINST_KEY}" "URLInfoAbout" "https://bogo.hannom.org/"
  WriteRegStr HKLM "${REG_UNINST_KEY}" "HelpLink" "https://bogo.hannom.org/docs/"
  WriteRegDWORD HKLM "${REG_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${REG_UNINST_KEY}" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; run as user...
  IfSilent deploy_silently
  ExecWait "$INSTDIR\NomIMEDeployer.exe /install"
  GoTo deploy_done

  deploy_silently:
  ExecWait "$INSTDIR\NomIMEDeployer.exe /deploy"
  deploy_done:

  ; don't redirect on 64 bit system for auto run setting
  ${If} ${IsNativeARM64}
    SetRegView 64
  ${ElseIf} ${IsNativeAMD64}
    SetRegView 64
  ${Endif}
  ; Write autorun key
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "NomIMEServer" "$INSTDIR\NomIMEServer.exe"
  ; Start NomIMEServer
  Exec "$INSTDIR\NomIMEServer.exe"

  ; option CheckForUpdates
  IfSilent DisableAutoCheckUpdate
  MessageBox MB_YESNO|MB_ICONINFORMATION "$(AUTOCHKUPDATE)" IDYES EnableAutoCheckUpdate
  DisableAutoCheckUpdate:
  WriteRegStr HKCU "Software\NomIME\NomIME\Updates" "CheckForUpdates" "0"
  GoTo end
  EnableAutoCheckUpdate:
  WriteRegStr HKCU "Software\NomIME\NomIME\Updates" "CheckForUpdates" "1"
  end:

  ; Prompt reboot
  StrCmp $0 "Upgrade" 0 +2
  SetRebootFlag true

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\$(DISPLAYNAME)"
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORMANUAL).lnk" "$INSTDIR\README.txt"
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORSETTING).lnk" "$INSTDIR\NomIMEDeployer.exe" "" "$SYSDIR\shell32.dll" 21
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORDICT).lnk" "$INSTDIR\NomIMEDeployer.exe" "/dict" "$SYSDIR\shell32.dll" 6
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORSYNC).lnk" "$INSTDIR\NomIMEDeployer.exe" "/sync" "$SYSDIR\shell32.dll" 26
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORDEPLOY).lnk" "$INSTDIR\NomIMEDeployer.exe" "/deploy" "$SYSDIR\shell32.dll" 144
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORSERVER).lnk" "$INSTDIR\NomIMEServer.exe" "" "$INSTDIR\NomIMEServer.exe" 0
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORUSERFOLDER).lnk" "$INSTDIR\NomIMEServer.exe" "/userdir" "$SYSDIR\shell32.dll" 126
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORAPPFOLDER).lnk" "$INSTDIR\NomIMEServer.exe" "/nomimedir" "$SYSDIR\shell32.dll" 19
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORUPDATER).lnk" "$INSTDIR\NomIMEServer.exe" "/update" "$SYSDIR\shell32.dll" 13
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORSETUP).lnk" "$INSTDIR\NomIMESetup.exe" "" "$SYSDIR\shell32.dll" 162
  CreateShortCut "$SMPROGRAMS\$(DISPLAYNAME)\$(LNKFORUNINSTALL).lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

  ExecWait '"$INSTDIR\NomIMEServer.exe" /quit'

  ExecWait '"$INSTDIR\NomIMESetup.exe" /u'

  ; Remove registry keys
  DeleteRegKey HKLM SOFTWARE\NomIME
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\NomIME"
  ; don't redirect on 64 bit system for auto run setting
  ${If} ${IsNativeARM64}
    SetRegView 64
  ${ElseIf} ${IsNativeAMD64}
    SetRegView 64
  ${Endif}
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "NomIMEServer"

  ; Remove files and uninstaller
  SetOutPath $TEMP
  Delete  "$INSTDIR\data\opencc\*.*"
  Delete  "$INSTDIR\data\preview\*.*"
  Delete  "$INSTDIR\data\*.*"
  Delete  "$INSTDIR\*.*"
  RMDir  "$INSTDIR\data\opencc"
  RMDir  "$INSTDIR\data\preview"
  RMDir  "$INSTDIR\data"
  RMDir  "$INSTDIR"
  SetShellVarContext all
  Delete  "$SMPROGRAMS\$(DISPLAYNAME)\*.*"
  RMDir  "$SMPROGRAMS\$(DISPLAYNAME)"

  ; Prompt reboot
  SetRebootFlag true

SectionEnd
