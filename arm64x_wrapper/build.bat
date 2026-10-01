
@echo off
setlocal

call :find_toolchain
if errorlevel 1 exit /b 1

echo Windows SDK Version = %WindowsSDKVersion:~0,-1%

REM Build dummy object files
cl.exe /c /Fo:dummy.o dummy.c || exit /b 1
cl.exe /c /arm64EC /Fo:dummy_x64.o dummy.c || exit /b 1

REM Build nomime.dll wrapper
link.exe /lib /machine:arm64ec /def:NomIMETSF_x64.def /out:NomIMETSF_x64.lib /ignore:4104 || exit /b 1
link.exe /lib /machine:arm64 /def:NomIMETSF_arm64.def /out:NomIMETSF_arm64.lib /ignore:4104 || exit /b 1
link.exe /dll /noentry /machine:arm64x /defArm64Native:NomIMETSF_arm64.def /def:NomIMETSF_x64.def ^
  /out:nomimeARM64X.dll dummy.o dummy_x64.o NomIMETSF_x64.lib NomIMETSF_arm64.lib /ignore:4104 || exit /b 1

endlocal
exit /b

:find_toolchain
set VSWHERE="%PROGRAMFILES%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% (
  set VSWHERE="%PROGRAMFILES(X86)%\Microsoft Visual Studio\Installer\vswhere.exe"
)
if not exist %VSWHERE% (
  echo vswhere not found, could not find a working MSVC install.
  exit /b 1
)

for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.ARM64EC -property installationPath`) do (
  set ARM64EC_TOOLCHAIN=%%i
)

if exist "%ARM64EC_TOOLCHAIN%\Common7\Tools\vsdevcmd.bat" (
  call "%ARM64EC_TOOLCHAIN%\Common7\Tools\vsdevcmd.bat" -arch=arm64 -host_arch=x86
  echo.
) else (
  echo ARM64EC capable toolchain not found
  exit /b 1
)

exit /b

