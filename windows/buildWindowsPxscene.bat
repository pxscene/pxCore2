cd ..
mkdir temp
cd temp
cmake ..
msbuild pxscene2d.sln /p:Configuration=Release /m
cd ..

rmdir /s /q Release
mkdir Release

xcopy /s /y /i examples\pxScene2d\src\browser Release\browser 
xcopy /s /y /i examples\pxScene2d\src\rcvrcore Release\rcvrcore 
xcopy /s /y examples\pxScene2d\src\*.js Release\
xcopy /s /y examples\pxScene2d\src\FreeSans.ttf Release\
xcopy /s /y examples\pxScene2d\src\*.json Release\
xcopy /s /y examples\pxScene2d\src\*.conf Release\
xcopy /s /y examples\pxScene2d\external\vc.build\builds\*.dll Release\
xcopy /s /y examples\pxScene2d\src\Release\* Release\

cd windows