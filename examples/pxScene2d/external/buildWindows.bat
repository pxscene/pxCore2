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

set buildExternal=0
if NOT [%APPVEYOR_REPO_COMMIT%] == [] (
    FOR /F "tokens=* USEBACKQ" %%F IN (`git diff --name-only %APPVEYOR_REPO_COMMIT% %APPVEYOR_REPO_COMMIT%~`) DO (
    echo.%%F|findstr "zlib WinSparkle pthread libpng libjpeg-turbo glew freetype curl jpeg-9a"
    if !errorlevel! == 0 (
      set buildExternal=1
      echo. External library files are modified. Need to build external : !buildExternal! .
      GOTO BREAK_LOOP1
    )
  )
)

@rem freetype latest version needs to be updated here. Because the lib is named based on version, so to avoid a build failure and to build the external when there is a difference in version.
cat vc.build\config.props | grep "freetype-2.8.1"
if !errorlevel! == 0 (
  if exist vc.build\builds\freetype281MT_D.lib ( 
    echo "freetpye cache available"
    ) ELSE (
    set buildExternal=1
	echo "Exact cache is not present, Externals will be built"
	)
  ) ELSE (
  set buildExternal=1
  echo "Exact library version is not present, Externals will be built"
)


if [%APPVEYOR_REPO_COMMIT%] == [] (
set buildExternal=1
)

:BREAK_LOOP1
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
patch -p1 < patches/add_ScaleXY.diff
cd ..

REM --------- LIBNODE

cd libnode-v6.9.0
CALL vcbuild.bat x86 nosign
cd ..

REM --------- DUKLUV

cd dukluv
patch -p1 < patches/dukluv.git.patch
mkdir build
cd build
cmake ..
cmake --build . --config Release -- /m
cd ..
