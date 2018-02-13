@echo on
@rem  Script to upload spark  (aka pxscene ) artifacts to build server) on
@rem  Windows platform (locally and on AppVeyor)
@rem
@rem
@rem  Assumes the following components are pre-installed:
@rem    - Visual Studio 2017,
@rem    - NSIS(>3.x), cmake(>2.8.x), python(2.7.x), 7z (all added to PATH).
@rem


cd %S%
cd "build-win32/_CPack_Packages/win32/NSIS"

set cronUpload=False

if "%APPVEYOR_SCHEDULED_BUILD%" == "True"  if "%DUKTAPE_SUPPORT%" == "ON" set cronUpload=True

if %cronUpload% == True (
    7z a -y pxscene-setup-exe.zip pxscene-setup.exe 
    echo y | "C:\Program Files\PuTTY\pscp.exe" -i %S%\pxscene-build.pem.ppk -P 2220 pxscene-setup-exe.zip "ubuntu@96.116.56.119:/var/www/html/edge/windows"
    IF %ERRORLEVEL% NEQ 0 (
      echo "-------------------------- Failed to upload pxscene setup"
	  EXIT 1
    )
    echo y | "C:\Program Files\PuTTY\pscp.exe" -i %S%\pxscene-build.pem.ppk -P 2220 %S%\logs\build_logs.txt "ubuntu@96.116.56.119:/var/www/html/edge/windows"
    IF %ERRORLEVEL% NEQ 0 (
      echo "-------------------------- Failed to upload build logs"
    )
)

if %cronUpload% == False (
  type %S%\logs\build_logs.txt
)

del %S%\pxscene-build.pem.ppk

EXIT 0
