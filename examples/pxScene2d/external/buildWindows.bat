@where msbuild 2> nul
@if %errorlevel% neq 0 (
 @echo Please execute this file from inside Visual Studio's Developer Command Prompt
 pause
 exit
)

copy /y libjpeg-turbo-1.5.1\win_temp\* libjpeg-turbo-1.5.1\
copy /y curl-7.40.0\include\curl\curlbuild-win.h curl-7.40.0\include\curl\curlbuild.h
copy /y libpng-1.6.28\scripts\pnglibconf.h.prebuilt libpng-1.6.28\pnglibconf.h
copy /y jpeg-9a\jconfig.vc jpeg-9a\jconfig.h


cd vc.build\
msbuild external.sln /m
cd ..

cd libnode-v6.9.0
vcbuild.bat x86 nosign
cd ..
