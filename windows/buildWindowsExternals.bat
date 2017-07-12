cd ..
xcopy /y examples\pxScene2d\external\libjpeg-turbo-1.5.1\win_temp\* examples\pxScene2d\external\libjpeg-turbo-1.5.1\
xcopy /y examples\pxScene2d\external\curl-7.40.0\include\curl\curlbuild-win.h examples\pxScene2d\external\curl-7.40.0\include\curl\curlbuild.h
xcopy /y examples\pxScene2d\external\libpng-1.6.28\scripts\pnglibconf.h.prebuilt examples\pxScene2d\external\libpng-1.6.28\pnglibconf.h
xcopy /y examples\pxScene2d\external\jpeg-9a\jconfig.vc examples\pxScene2d\external\jpeg-9a\jconfig.h


cd examples\pxScene2d\external\vc.build\
msbuild external.sln /m
cd ..\..\..\..\

cd examples\pxScene2d\external\libnode-v6.9.0
vcbuild.bat x86 nosign
cd ..\..\..\..\

cd windows