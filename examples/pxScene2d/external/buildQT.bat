@echo off
echo "QT must be build under visual studio x86 x64 cross tools command prompt"
set "ZIP=qt-everywhere-src-5.12.0-win.zip"
set "QT_FOLDER=qt-everywhere-src-5.12.0"
SET "ZIPNAME=%~dp0\%ZIP%"
if EXIST "%ZIP%" (
    echo "%ZIP% exist, skip download"
) else ( 
    echo "start download %ZIP% ..."
    bitsadmin.exe /transfer "downloadQT" /PRIORITY high http://download.qt.io/official_releases/qt/5.12/5.12.0/single/qt-everywhere-src-5.12.0.zip "%ZIPNAME%"
    echo "download done."
)

if EXIST "%QT_FOLDER%" (
    echo "%QT_FOLDER% exist, skip unzip"
) else (
     echo "start unzip %ZIP% ..."
     unzip "%ZIP%"
     echo "unzip done."
)


echo "start download %ZIP% ..."
del /f jom_1_1_3.zip
rd /s /q jom
bitsadmin.exe /transfer "downloadJom" /PRIORITY high http://download.qt.io/official_releases/jom/jom_1_1_3.zip %~dp0\jom_1_1_3.zip"
unzip jom_1_1_3.zip -d jom

SET _ROOT="%~dp0\%QT_FOLDER%"
echo "QT root = %_ROOT%"

set PATH=%~dp0\jom;%PATH%
SET PATH=%_ROOT%\qtbase\bin;%_ROOT%\gnuwin32\bin;%PATH%
SET PATH=%_ROOT%\qtrepotools\bin;%PATH%

cd "%QT_FOLDER%"
call configure -developer-build -opensource -nomake examples -nomake tests

jom

cd qtwebengine
jom

cd ..\

cd qtbase
rd /s /q dist
mkdir dist

copy .\lib\Qt5Core.dll .\dist
copy .\lib\Qt5Gui.dll .\dist
copy .\lib\Qt5Network.dll .\dist
copy .\lib\Qt5Positioning.dll .\dist
copy .\lib\Qt5PrintSupport.dll .\dist
copy .\lib\Qt5Qml.dll .\dist
copy .\lib\Qt5Quick.dll .\dist
copy .\lib\Qt5QuickWidgets.dll .\dist
copy .\lib\Qt5WebChannel.dll .\dist
copy .\lib\Qt5WebEngineCore.dll .\dist
copy .\lib\Qt5WebEngineWidgets.dll .\dist
copy .\lib\Qt5Widgets.dll .\dist
copy .\lib\libEGL.dll .\dist
copy .\lib\libGLESv2.dll .\dist
copy .\bin\QtWebEngineProcess.exe .\dist

xcopy .\translations\qtwebengine_locales .\dist\qtwebengine_locales\

copy .\resources\* .\dist

mkdir dist\platforms

copy .\plugins\platforms\qwindows.dll .\dist\platforms

cd ..\..\

setx QT_HOME "%~dp0\%QT_FOLDER%"