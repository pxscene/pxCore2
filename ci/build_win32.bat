@echo on
@rem  Script to build spark (aka pxscene) on
@rem  Windows platform (locally and on AppVeyor)
@rem
@rem  Author: Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
@rem
@rem
@rem  Assumes the following components are pre-installed:
@rem    - Visual Studio 2017,
@rem    - NSIS(>3.x), cmake(>2.8.x), python(2.7.x), 7z (all added to PATH).
@rem

cmake --version

@rem Use python2.7 on Azure (https://github.com/Microsoft/azure-pipelines-image-generation/blob/master/images/win/Vs2017-Server2016-Readme.md)
if exist c:\python27amd64 set PATH=c:\python27amd64;%PATH%
python --version

@rem On Azure make sure cmake is before "C:\ProgramData\Chocolatey\bin"
@rem otherwise cpack.exe will be used from Chocolatey
if exist "C:\Program Files\CMake\bin" set PATH=C:\Program Files\CMake\bin;%PATH%

set "ORIG_DIR=%CD%"

cd %~dp0
cd ..
set "BASE_DIR=%CD%"

set "VSCMD_START_DIR=%CD%"
call "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/VC/Auxiliary/Build/vcvars32.bat" x86
if %errorlevel% neq 0 call "C:/Program Files (x86)/Microsoft Visual Studio/2017/Enterprise/VC/Auxiliary/Build/vcvars32.bat" x86

@rem build dependencies
cd examples/pxScene2d/external
call buildWindows.bat
if %errorlevel% neq 0 exit /b %errorlevel%

@rem Avoid using link.exe from that paths
set PATH=%PATH:C:\Program Files\Git\usr\bin;=%
set PATH=%PATH:c:\Program Files\Git\usr\bin;=%
set PATH=%PATH:C:\cygwin64\bin;=%
set PATH=%PATH:c:\cygwin64\bin;=%

cd "%BASE_DIR%"
md build-win32
cd build-win32
set addVer=False
set uploadArtifact=False
@rem build Spark
if "%APPVEYOR_SCHEDULED_BUILD%"=="True" (
  echo "building edge"
  copy ..\examples\pxScene2d\src\browser\images\status_bg_edge.svg ..\examples\pxScene2d\src\browser\images\status_bg.svg

  set uploadArtifact=True
  call:replaceString "..\examples\pxScene2d\src\win\pxscene.rc" "Spark_installer.ico" "SparkEdge_installer.ico"
  cmake -DSUPPORT_DUKTAPE=OFF -DCMAKE_VERBOSE_MAKEFILE=ON -DPXSCENE_VERSION=edge_%date:~-4%-%date:~4,2%-%date:~7,2% ..
  call:replaceString "examples\pxScene2d\src\cmake_install.cmake" "Spark.exe" "SparkEdge.exe"
  call:replaceString "CPackConfig.cmake" "Spark.exe" "SparkEdge.exe"
  call:replaceString "CPackSourceConfig.cmake" "Spark.exe" "SparkEdge.exe"
  call:replaceString "CPackConfig.cmake" ""Spark"" ""SparkEdge""
  call:replaceString "CPackSourceConfig.cmake" ""Spark"" ""SparkEdge""
  call:replaceString "CPackConfig.cmake" "Spark.lnk" "SparkEdge.lnk"
  call:replaceString "CPackSourceConfig.cmake" "Spark.lnk" "SparkEdge.lnk"
  call:replaceString "CPackConfig.cmake" "Spark_installer.ico" "SparkEdge_installer.ico"
  call:replaceString "CPackSourceConfig.cmake" "Spark_installer.ico" "SparkEdge_installer.ico"
  )

del ..\examples\pxScene2d\src\browser\images\status_bg_edge.svg 


for /f "tokens=1,* delims=]" %%a in ('find /n /v "" ^< "..\examples\pxScene2d\src\win\pxscene.rc" ^| findstr "FILEVERSION" ') DO ( 
			call set verInfo=%%b
	)
	call set verInfo=%verInfo:~12%
	call set verInfo=%verInfo:,=.%
		
	if "%APPVEYOR_FORCED_BUILD%"=="True" set uploadArtifact=True
	if "%APPVEYOR_REPO_TAG%"=="true" set uploadArtifact=True
	
	if  "%APPVEYOR_SCHEDULED_BUILD%"=="" (
		if "%uploadArtifact%"=="True" cmake -DSUPPORT_DUKTAPE=OFF -DCMAKE_VERBOSE_MAKEFILE=ON -DPXSCENE_VERSION=%verInfo% .. 
		if "%uploadArtifact%"=="False"  cmake -DCMAKE_VERBOSE_MAKEFILE=ON .. 
	)
	
echo Building Spark...
cmake --build . --config Release -- /m
if %errorlevel% neq 0 exit /b %errorlevel%

if "%APPVEYOR_SCHEDULED_BUILD%"=="True" (
move ..\examples\pxScene2d\src\Release\Spark.exe ..\examples\pxScene2d\src\Release\SparkEdge.exe
)

echo Building installer...
cpack --verbose .
if %errorlevel% neq 0  (
  type _CPack_Packages\win32\NSIS\NSISOutput.log
  exit /b %errorlevel% 
)


echo Creating standalone archive
cd _CPack_Packages/win32/NSIS
7z a -y spark-setup.zip spark-setup

cd %ORIG_DIR%

@rem deploy artifacts
@rem based on: https://www.appveyor.com/docs/build-worker-api/#push-artifact
echo.uploadArtifact : %uploadArtifact%
if "%uploadArtifact%" == "True" (

        @rem NSIS based installer
        appveyor PushArtifact "build-win32\\_CPack_Packages\\win32\\NSIS\\spark-setup.exe" -DeploymentName "installer" -Type "Auto" -Verbosity "Normal"

        @rem Standalone (requires no installation)
        appveyor PushArtifact "build-win32\\_CPack_Packages\\win32\\NSIS\\spark-setup.zip" -DeploymentName "portable" -Type "Zip" -Verbosity "Normal"
)

GOTO scriptEnd

:replaceString <fileName>
set INTEXTFILE=%~1
set OUTTEXTFILE=%~1.mod
set SEARCHTEXT=%~2
set REPLACETEXT=%~3
for /f "tokens=1,* delims=¶" %%A in ( '"type %INTEXTFILE%"') do (
    SET string=%%A
        setlocal EnableDelayedExpansion
            SET modified=!string:%SEARCHTEXT%=%REPLACETEXT%!

                >> %OUTTEXTFILE% echo(!modified!
                    endlocal
                    )
                    del %INTEXTFILE%
                    copy %OUTTEXTFILE% %INTEXTFILE%
                    del %OUTTEXTFILE%
goto:eof

:ScriptEnd
@rem exit 0
