@echo off
setlocal

set RELEASE_VERSION=20261026

set JOBS=8
set SCONS=python3 -m SCons
set ZIP7="C:\Program Files\7-Zip\7z"
set SAIGO_DIR=toolchain\saigosdk-windows-amd64_20260925

:: Must set MSYS_DIR on command line. The MinGW distro doesn't really have to be MSYS
:: but this makes it more convenient to set just one variable.
set MINGW_AMD64="%MSYS_DIR%\mingw64"
set MINGW_I686="%MSYS_DIR%\mingw32"

%MINGW_AMD64%\bin\as --version
%MINGW_I686%\bin\as --version
%ZIP7%
python3 -m pip freeze

rd /S /Q scons-out release

git log -1
git status

%SCONS% -j%JOBS% --mode=opt-win,nacl_irt saigo_newlib_dir=%SAIGO_DIR% mingw_dir=%MINGW_AMD64% platform=x86-64 saigo=1 --no-clang werror=0 sysinfo=0 sel_ldr irt_core || exit /B
%SCONS% -j%JOBS% --mode=opt-win,nacl_irt saigo_newlib_dir=%SAIGO_DIR% mingw_dir=%MINGW_I686% platform=x86-32 saigo=1 --no-clang werror=0 sysinfo=0 sel_ldr irt_core || exit /B

mkdir release
cd release

set TGT64=naclruntime-windows-amd64_%RELEASE_VERSION%
set TGT32=naclruntime-windows-i686_%RELEASE_VERSION%
mkdir %TGT64%
mkdir %TGT32%
copy ..\scons-out\opt-win-x86-64\staging\sel_ldr.exe %TGT64%
copy ..\scons-out\opt-win-x86-32\staging\sel_ldr.exe %TGT32%
copy ..\scons-out\nacl_irt-x86-64\staging\irt_core.nexe %TGT64%
copy ..\scons-out\nacl_irt-x86-32\staging\irt_core.nexe %TGT32%
%ZIP7% a -tzip -mx=9 %TGT64%.zip %TGT64% || exit /B
%ZIP7% a -tzip -mx=9 %TGT32%.zip %TGT32% || exit /B
