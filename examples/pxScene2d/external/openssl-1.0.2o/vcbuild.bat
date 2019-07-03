REM -------- OPENSSL
IF "%APPVEYOR_BUILD_ID%" == "" GOTO SETENV
:SETENV
rem set msvs_host_arch=x86
rem if _%PROCESSOR_ARCHITECTURE%_==_AMD64_ set msvs_host_arch=amd64
rem if _%PROCESSOR_ARCHITEW6432%_==_AMD64_ set msvs_host_arch=amd64
rem set vcvarsall_arg=%msvs_host_arch%_%target_arch%
rem call ..\libnode-v10.15.3\tools\msvs\vswhere_usability_wrapper.cmd
rem set "VSINSTALLDIR="
rem set "VSCMD_START_DIR=%CD%"
rem set vcvars_call="%VCINSTALLDIR%\Auxiliary\Build\vcvarsall.bat" %vcvarsall_arg%
rem echo calling: %vcvars_call%
rem call %vcvars_call%
GOTO BUILDSSL
:BUILDSSL
perl Configure VC-WIN32 no-asm --prefix=%CD%
call ms\do_ms
nmake /f ms\ntdll.mak clean
nmake /f ms\ntdll.mak
nmake /f ms\ntdll.mak install
