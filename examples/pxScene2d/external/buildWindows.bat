@where msbuild 2> nul
@if %errorlevel% neq 0 (
 @echo.
 @echo Please execute this file from inside Visual Studio's Developer Command Prompt
 @echo.
 pause
 goto :eof
)
@echo off
setlocal enabledelayedexpansion

copy /y libjpeg-turbo-1.5.1\win_temp\* libjpeg-turbo-1.5.1\
copy /y curl-7.40.0\include\curl\curlbuild-win.h curl-7.40.0\include\curl\curlbuild.h
copy /y libpng-1.6.28\scripts\pnglibconf.h.prebuilt libpng-1.6.28\pnglibconf.h
copy /y jpeg-9a\jconfig.vc jpeg-9a\jconfig.h

REM --------- SQLITE

cd sqlite-autoconf-3280000
cl /c /EHsc sqlite3.c
lib sqlite3.obj
cd ..

REM --------- GIF
cd giflib-5.1.9
REM patch -p1 < ../giflib-5.1.9-windows.diff
git apply --ignore-space-change --ignore-whitespace --whitespace=nowarn ../giflib-5.1.9-windows.diff

cl /c /EHsc dgif_lib.c egif_lib.c getarg.c gif2rgb.c gif_err.c gif_font.c gif_hash.c gifalloc.c gifbg.c gifbuild.c gifclrmp.c gifcolor.c gifecho.c giffilter.c giffix.c gifhisto.c gifinto.c gifsponge.c giftext.c giftool.c gifwedge.c openbsd-reallocarray.c qprintf.c quantize.c

lib *.obj

copy /y dgif_lib.lib libgif.7.lib
cd ..

set buildExternal=0
if NOT [%APPVEYOR_REPO_COMMIT%] == [] (
    FOR /F "tokens=* USEBACKQ" %%F IN (`git diff --name-only %APPVEYOR_REPO_COMMIT% %APPVEYOR_REPO_COMMIT%~`) DO (
    echo.%%F|findstr "openssl zlib WinSparkle pthread libpng libjpeg-turbo glew freetype curl jpeg-9a"
    if !errorlevel! == 0 (
      set buildExternal=1
      echo. External library files are modified. Need to build external : !buildExternal! .
      GOTO BREAK_LOOP1
    )
  )
)

@rem build openssl
cd openssl-1.0.2o
  CALL vcbuild.bat
cd ..

@rem freetype latest version needs to be updated here. Because the lib is named based on version, so to avoid a build failure and to build the external when there is a difference in version.
set buildExternal=1


if [%APPVEYOR_REPO_COMMIT%] == [] (
set buildExternal=1
)

:BREAK_LOOP1
@rem build openssl
cd openssl-1.0.2o
  CALL vcbuild.bat
cd ..
cd vc.build
if NOT EXIST builds (
  set buildExternal=1
  echo Cache not available. Need to build external : !buildNeeded!.
)
cd ..
if %buildExternal% == 1 (
  echo. Building external library  : %cd%
  cd vc.build\
  msbuild external.sln /p:Configuration=Release /p:Platform=Win32 /m
  cd ..
)

REM --------- BREAKPAD

REM todo uncomment - if you want to build v8 then call buildV8.bat directly
REM CALL buildV8.bat

cd breakpad-chrome_55
CALL gyp\gyp.bat src\client\windows\breakpad_client.gyp --no-circular-check
cd src\client\windows
msbuild breakpad_client.sln /p:Configuration=Release /p:Platform=Win32 /m
cd ..\..\..\..\

REM --------- NANOSVG

cd nanosvg
REM patch -p1 < patches/add_ScaleXY.diff
git apply --ignore-space-change --ignore-whitespace --whitespace=nowarn patches/add_ScaleXY.diff
cd ..

REM --------- LIBNODE

git apply --ignore-space-change --ignore-whitespace --whitespace=nowarn node-v10.15.3_mods.patch
git apply --ignore-space-change --ignore-whitespace --whitespace=nowarn openssl_1.0.2_compatibility.patch
cd libnode-v10.15.3
if %buildExternal% == 1 (
  CALL vcbuild.bat x86 nosign static openssl-no-asm shared-openssl
) else (
  CALL vcbuild.bat x86 nosign openssl-no-asm shared-openssl
)
cd ..

REM --------- DUKLUV

cd dukluv
REM patch -p1 < patches/dukluv.git.patch
git apply --ignore-space-change --ignore-whitespace --whitespace=nowarn patches/dukluv.git.patch
mkdir build
cd build
cmake ..
cmake --build . --config Release -- /m
cd ..

REM --------- GIF
cd giflib-5.1.9
git apply -p1 < ../giflib-5.1.9-windows.diff

cd ..

REM --------- SQLITE

cd sqlite-autoconf-3280000
cl /c /EHsc sqlite3.c
lib sqlite3.obj
cd ..
