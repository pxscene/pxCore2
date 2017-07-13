@where msbuild 2> nul
@if %errorlevel% neq 0 (
 @echo Please execute this file from inside the Developer Command Prompt for Visual Studio
 pause
 exit
)

@if not exist "examples\pxScene2d\external\libnode-v6.9.0\Release\node.exe" (
  echo Externals must be built first.  Please run examples\pxScene2d\external\buildWindows.bat first
  pause
  exit
) else (
  mkdir temp
  cd temp
  cmake ..
  msbuild pxscene2d.sln /p:Configuration=Release /m
  cd ..

  rmdir /s /q Release
  mkdir Release

  xcopy /s /y /i examples\pxScene2d\src\browser Release\browser 
  xcopy /s /y /i examples\pxScene2d\src\node_modules Release\node_modules 
  xcopy /s /y /i examples\pxScene2d\src\rcvrcore Release\rcvrcore 
  copy /y examples\pxScene2d\src\*.js Release\
  copy /y examples\pxScene2d\src\FreeSans.ttf Release\
  copy /y examples\pxScene2d\src\*.json Release\
  copy /y examples\pxScene2d\src\*.conf Release\
  copy /y examples\pxScene2d\external\vc.build\builds\*.dll Release\
  copy /y examples\pxScene2d\src\Release\* Release\
)

@if exist "Release\pxscene.exe" (
  echo Build Success
) else (
  echo Build Failure
)
  


