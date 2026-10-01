@echo off

setlocal

if not exist env.bat copy env.bat.template env.bat

if exist env.bat call env.bat

if not defined NOMIME_ROOT set NOMIME_ROOT=%CD%

if not defined VERSION_MAJOR set VERSION_MAJOR=1
if not defined VERSION_MINOR set VERSION_MINOR=0
if not defined VERSION_PATCH set VERSION_PATCH=7

if not defined NOMIME_VERSION set NOMIME_VERSION=%VERSION_MAJOR%.%VERSION_MINOR%.%VERSION_PATCH%
if not defined NOMIME_BUILD set NOMIME_BUILD=0

rem use numeric build version for release build
set PRODUCT_VERSION=%NOMIME_VERSION%.%NOMIME_BUILD%
rem for non-release build, try to use git commit hash as product build version
if not defined RELEASE_BUILD (
  rem check if git is installed and available, then get the short commit id of head
  git --version >nul 2>&1
  if not errorlevel 1 (
    for /f "delims=" %%i in ('git tag --sort=-creatordate ^| findstr /r "%NOMIME_VERSION%"') do (
      set LAST_TAG=%%i
      goto found_tag
    )
    :found_tag
    for /f "delims=" %%i in ('git rev-list %LAST_TAG%..HEAD --count') do (
      set NOMIME_BUILD=%%i
    )
    set PRODUCT_VERSION=%NOMIME_VERSION%.%NOMIME_BUILD%
  )
)

rem FILE_VERSION is always 4 numbers; same as PRODUCT_VERSION in release build
if not defined FILE_VERSION set FILE_VERSION=%NOMIME_VERSION%.%NOMIME_BUILD%

echo PRODUCT_VERSION=%PRODUCT_VERSION%
echo NOMIME_VERSION=%NOMIME_VERSION%
echo NOMIME_BUILD=%NOMIME_BUILD%
echo NOMIME_ROOT=%NOMIME_ROOT%
echo NOMIME_BUNDLED_RECIPES=%NOMIME_BUNDLED_RECIPES%
echo.

if defined GITHUB_ENV (
	setlocal enabledelayedexpansion
	echo git_ref_name=%PRODUCT_VERSION%>>!GITHUB_ENV!
)

if defined BOOST_ROOT (
  if exist "%BOOST_ROOT%\boost" goto boost_found
)
echo Error: Boost not found! Please set BOOST_ROOT in env.bat.
exit /b 1

:boost_found
echo BOOST_ROOT=%BOOST_ROOT%
echo.

if not defined BJAM_TOOLSET (
  rem the number actually means platform toolset, not %VisualStudioVersion%
  set BJAM_TOOLSET=msvc-14.2
)

if not defined PLATFORM_TOOLSET (
  set PLATFORM_TOOLSET=v142
)

if defined DEVTOOLS_PATH set PATH=%DEVTOOLS_PATH%%PATH%

set build_config=Release
set build_option=/t:Build
set build_boost=0
set boost_build_variant=release
set build_data=0
set build_opencc=0
set build_rime=0
set rime_build_variant=release
set build_nomime=0
set build_installer=0
set build_arm64=0

rem parse the command line options
:parse_cmdline_options
  if "%1" == "" goto end_parsing_cmdline_options
  if "%1" == "debug" (
    set build_config=Debug
    set boost_build_variant=debug
    set rime_build_variant=debug
  )
  if "%1" == "release" (
    set build_config=Release
    set boost_build_variant=release
    set rime_build_variant=release
  )
  if "%1" == "rebuild" set build_option=/t:Rebuild
  if "%1" == "boost" set build_boost=1
  if "%1" == "data" set build_data=1
  if "%1" == "opencc" set build_opencc=1
  if "%1" == "rime" set build_rime=1
  if "%1" == "librime" set build_rime=1
  if "%1" == "nomime" set build_nomime=1
  if "%1" == "installer" set build_installer=1
  if "%1" == "arm64" set build_arm64=1
  if "%1" == "all" (
    set build_boost=1
    set build_data=1
    set build_opencc=1
    set build_rime=1
    set build_nomime=1
    set build_installer=1
    set build_arm64=1
  )
  shift
  goto parse_cmdline_options
:end_parsing_cmdline_options

if %build_nomime% == 0 (
if %build_boost% == 0 (
if %build_data% == 0 (
if %build_opencc% == 0 (
if %build_rime% == 0 (
  set build_nomime=1
)))))

rem quit NomIMEServer.exe before building
cd /d %NOMIME_ROOT%
if exist output\nomimeserver.exe (
  output\nomimeserver.exe /q
)

rem build booost
if %build_boost% == 1 (
  call :build_boost
  if errorlevel 1 exit /b 1
  cd /d %NOMIME_ROOT%
)

rem -------------------------------------------------------------------------
rem build librime x64 and Win32
if %build_rime% == 1 (
  if not exist librime\build.bat (
    git submodule update --init --recursive
  )
  cd %NOMIME_ROOT%\librime
  rem clean cache before building
  for %%a in ( build dist lib ^
    deps\glog\build ^
    deps\googletest\build ^
    deps\leveldb\build ^
    deps\marisa-trie\build ^
    deps\opencc\build ^
    deps\yaml-cpp\build ) do (
      if exist %%a rd /s /q %%a
  )

  rem build x64 librime
  set ARCH=x64
  call :build_librime_platform x64 "%NOMIME_ROOT%\lib64" "%NOMIME_ROOT%\output"
  if errorlevel 1 goto error
  rem build Win32 librime
  set ARCH=Win32
  call :build_librime_platform Win32 "%NOMIME_ROOT%\lib" "%NOMIME_ROOT%\output\Win32"
  if errorlevel 1 goto error
  rem clean the modified file
  rem git checkout .
  rem git submodule foreach git checkout .
)

rem -------------------------------------------------------------------------
rem The librime steps above leave the working directory inside librime\, and
rem the checks and copies below are relative to the repository root.
cd /d "%NOMIME_ROOT%"
if %build_nomime% == 1 (
  if not exist output\data\essay.txt (
    set build_data=1
  )
  if not exist output\data\opencc\TSCharacters.ocd* (
    set build_opencc=1
  )
)
if %build_data% == 1 call :build_data
if %build_opencc% == 1 call :build_opencc_data

if %build_nomime% == 0 goto end

cd /d %NOMIME_ROOT%

set NOMIME_PROJECT_PROPERTIES=BOOST_ROOT^
  PLATFORM_TOOLSET^
  VERSION_MAJOR^
  VERSION_MINOR^
  VERSION_PATCH^
  PRODUCT_VERSION^
  FILE_VERSION

cscript.exe render.js nomime.props %NOMIME_PROJECT_PROPERTIES%

del msbuild*.log

if defined SDKVER set build_sdk_option=/p:WindowsTargetPlatformVersion=%SDKVER%
if not defined SDKVER set build_sdk_option=

rem The .rc files of NomIMEServer and NomIMEDeployer embed build_out\SinoNom.bin,
rem so the vault has to be sealed before msbuild runs. It needs nomime.props,
rem which render.js has just written. A failure must stop the build: carrying on
rem would silently embed whatever stale SinoNom.bin is left from an earlier run.
call :pack_vault
if errorlevel 1 goto error

if %build_arm64% == 1 (

  msbuild.exe nomime.sln %build_option% /p:Configuration=%build_config% /p:Platform="ARM64" /fl5 %build_sdk_option%
  if errorlevel 1 goto error
)

msbuild.exe nomime.sln %build_option% /p:Configuration=%build_config% /p:Platform="x64" /fl2 %build_sdk_option%
if errorlevel 1 goto error
msbuild.exe nomime.sln %build_option% /p:Configuration=%build_config% /p:Platform="Win32" /fl1 %build_sdk_option%
if errorlevel 1 goto error

if %build_arm64% == 1 (
  pushd arm64x_wrapper
  call build.bat
  if errorlevel 1 goto error
  popd

  copy arm64x_wrapper\nomimeARM64X.dll output
  if errorlevel 1 goto error
)

if %build_installer% == 1 (
  "%ProgramFiles(x86)%"\NSIS\Bin\makensis.exe ^
  /DNOMIME_VERSION=%NOMIME_VERSION% ^
  /DNOMIME_BUILD=%NOMIME_BUILD% ^
  /DPRODUCT_VERSION=%PRODUCT_VERSION% ^
  output\install.nsi
  if errorlevel 1 goto error
)

goto end

rem -------------------------------------------------------------------------
rem build boost
:build_boost
  set BJAM_OPTIONS_COMMON=-j%NUMBER_OF_PROCESSORS%^
    --with-filesystem^
    --with-json^
    --with-locale^
    --with-regex^
    --with-serialization^
    --with-thread^
    define=BOOST_USE_WINAPI_VERSION=0x0603^
    toolset=%BJAM_TOOLSET%^
    link=static^
    runtime-link=static^
    --build-type=complete
  
  set BJAM_OPTIONS_X86=%BJAM_OPTIONS_COMMON%^
    architecture=x86^
    address-model=32
  
  set BJAM_OPTIONS_X64=%BJAM_OPTIONS_COMMON%^
    architecture=x86^
    address-model=64
  
  set BJAM_OPTIONS_ARM32=%BJAM_OPTIONS_COMMON%^
    define=BOOST_USE_WINAPI_VERSION=0x0A00^
    architecture=arm^
    address-model=32
  
  set BJAM_OPTIONS_ARM64=%BJAM_OPTIONS_COMMON%^
    define=BOOST_USE_WINAPI_VERSION=0x0A00^
    architecture=arm^
    address-model=64
  
  cd /d %BOOST_ROOT%
  if not exist b2.exe call bootstrap.bat
  if errorlevel 1 goto error
  b2 %BJAM_OPTIONS_X86% stage %BOOST_COMPILED_LIBS%
  if errorlevel 1 goto error
  b2 %BJAM_OPTIONS_X64% stage %BOOST_COMPILED_LIBS%
  if errorlevel 1 goto error
  
  if %build_arm64% == 1 (
    b2 %BJAM_OPTIONS_ARM64% stage %BOOST_COMPILED_LIBS%
    if errorlevel 1 goto error
  )
  exit /b

rem ---------------------------------------------------------------------------
:build_data
  copy "%NOMIME_ROOT%\LICENSE.txt" output\
  copy "%NOMIME_ROOT%\README.md" output\README.txt
  copy "%NOMIME_ROOT%\plum\rime-install.bat" output\
  set plum_dir=plum
  set rime_dir=output/data
  set WSLENV=plum_dir:rime_dir
  bash plum/rime-install %NOMIME_BUNDLED_RECIPES%
  if errorlevel 1 goto error
  exit /b

rem ---------------------------------------------------------------------------
rem Seals the SinoNom schema and dictionary into build_out\SinoNom.bin, which
rem NomIMEServer and NomIMEDeployer embed as RCDATA. The plain sources stay in
rem data_src\ and are never copied into output\data.
:pack_vault
  rem Every path is quoted: NOMIME_ROOT may contain spaces.
  set "vault_src=%NOMIME_ROOT%\data_src"
  set "vault_tmp=%NOMIME_ROOT%\build_out\vault"
  set "vault_bin=%NOMIME_ROOT%\build_out\SinoNom.bin"
  set "vault_rime=%NOMIME_ROOT%\librime\dist_x64"
  if not exist "%vault_src%\SinoNom.dict.yaml" (
    echo ERROR: data_src\SinoNom.dict.yaml not found; cannot seal the data vault.
    goto error
  )
  if not exist "%vault_rime%\bin\rime_deployer.exe" (
    echo ERROR: rime_deployer.exe not found; build librime before sealing the vault.
    goto error
  )
  msbuild.exe "%NOMIME_ROOT%\NomIMEDataPacker\NomIMEDataPacker.vcxproj" ^
    /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo %build_sdk_option%
  if errorlevel 1 goto error

  rem Compile in a scratch tree, so no plain dictionary ever reaches output\data.
  if exist "%vault_tmp%" rmdir /s /q "%vault_tmp%"
  mkdir "%vault_tmp%\shared"
  mkdir "%vault_tmp%\user"
  mkdir "%vault_tmp%\build"
  mkdir "%vault_tmp%\bin"
  copy /Y "%NOMIME_ROOT%\output\data\*.yaml" "%vault_tmp%\shared\" >nul
  copy /Y "%vault_src%\*.yaml" "%vault_tmp%\shared\" >nul
  copy /Y "%vault_rime%\bin\rime_deployer.exe" "%vault_tmp%\bin\" >nul
  copy /Y "%vault_rime%\lib\rime.dll" "%vault_tmp%\bin\" >nul

  "%vault_tmp%\bin\rime_deployer.exe" --compile ^
    "%vault_tmp%\shared\SinoNom.schema.yaml" ^
    "%vault_tmp%\user" ^
    "%vault_tmp%\shared" ^
    "%vault_tmp%\build"
  if errorlevel 1 goto pack_vault_failed

  "%NOMIME_ROOT%\build_out\tools\NomIMEDataPacker.exe" pack "%vault_bin%" ^
    "%vault_tmp%\build\SinoNom.schema.yaml" ^
    "%vault_tmp%\build\SinoNom.table.bin" ^
    "%vault_tmp%\build\SinoNom.prism.bin" ^
    "%vault_tmp%\build\SinoNom.reverse.bin"
  if errorlevel 1 goto pack_vault_failed

  rem The scratch tree holds the unsealed tables and rime_deployer's .txt dumps
  rem of the entire dictionary, so it must not survive the build.
  rmdir /s /q "%vault_tmp%"
  exit /b 0

:pack_vault_failed
  rem Same reason: a failed build must not leave the plain dictionary behind.
  if exist "%vault_tmp%" rmdir /s /q "%vault_tmp%"
  goto error

rem ---------------------------------------------------------------------------
:build_opencc_data
  if not exist "%NOMIME_ROOT%\librime\share\opencc\TSCharacters.ocd2" (
    cd %NOMIME_ROOT%\librime
    call build.bat deps %rime_build_variant%
    if errorlevel 1 goto error
  )
  cd %NOMIME_ROOT%
  if not exist output\data\opencc mkdir output\data\opencc
  copy "%NOMIME_ROOT%\librime\share\opencc\*.*" output\data\opencc\
  if errorlevel 1 goto error
  exit /b

rem ---------------------------------------------------------------------------
rem %1 : ARCH
rem %2 : push | pop , push to backup when pop to restore
:stash_build
  pushd "%NOMIME_ROOT%\librime"
  for %%a in ( build dist lib ^
    deps\glog\build ^
    deps\googletest\build ^
    deps\leveldb\build ^
    deps\marisa-trie\build ^
    deps\opencc\build ^
    deps\yaml-cpp\build ) do (
    if "%2"=="push" (
      if exist %%a  move %%a %%a_%1 
    )
    if "%2"=="pop" (
      if exist %%a_%1  move %%a_%1 %%a 
    )
  )
  popd
  exit /b

rem ---------------------------------------------------------------------------
rem %1 : ARCH
rem %2 : target_path of rime.lib, base %NOMIME_ROOT% or abs path
rem %3 : target_path of rime.dll, base %NOMIME_ROOT% or abs path
:build_librime_platform
  rem restore backuped %1 build
  call :stash_build %1 pop

  cd %NOMIME_ROOT%\librime
  if not exist env.bat (
    copy "%NOMIME_ROOT%\env.bat" env.bat
  )
  if not exist lib\opencc.lib (
    call build.bat deps %rime_build_variant%
    if errorlevel 1 (
      call :stash_build %1 push
      goto error
    )
  )
  call build.bat %rime_build_variant%
  if errorlevel 1 (
    call :stash_build %1 push
    goto error
  )

  cd %NOMIME_ROOT%\librime
  call :stash_build %1 push

  rem Quoted throughout: NOMIME_ROOT may contain spaces. %~2 / %~3 strip the
  rem quotes the caller wraps those paths in.
  copy /Y "%NOMIME_ROOT%\librime\dist_%1\include\rime_*.h" "%NOMIME_ROOT%\include\"
  if errorlevel 1 goto error
  copy /Y "%NOMIME_ROOT%\librime\dist_%1\lib\rime.lib" "%~2\"
  if errorlevel 1 goto error
  copy /Y "%NOMIME_ROOT%\librime\dist_%1\lib\rime.dll" "%~3\"
  if errorlevel 1 goto error

  exit /b
rem ---------------------------------------------------------------------------

:error

cd %NOMIME_ROOT%
echo error building nomime...
exit /b 1

:end
cd %NOMIME_ROOT%
