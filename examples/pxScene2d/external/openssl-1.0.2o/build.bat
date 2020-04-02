REM -------- OPENSSL
cd openssl-1.0.2o
@rem Set environment for msbuild
set msvs_host_arch=x86
if _%PROCESSOR_ARCHITECTURE%_==_AMD64_ set msvs_host_arch=amd64
if _%PROCESSOR_ARCHITEW6432%_==_AMD64_ set msvs_host_arch=amd64
@rem usually vcvarsall takes an argument: host + '_' + target
set vcvarsall_arg=%msvs_host_arch%_%target_arch%
@rem unless both host and target are x64
if %target_arch%==x64 if %msvs_host_arch%==amd64 set vcvarsall_arg=amd64
@rem also if both are x86
if %target_arch%==x86 if %msvs_host_arch%==x86 set vcvarsall_arg=x86
call tools\msvs\vswhere_usability_wrapper.cmd
set "VSINSTALLDIR="
set "VSCMD_START_DIR=%CD%"
set vcvars_call="%VCINSTALLDIR%\Auxiliary\Build\vcvarsall.bat" %vcvarsall_arg%
echo calling: %vcvars_call%
call %vcvars_call%
perl Configure VC-WIN32 no-asm --prefix=%CD%
call ms\do_ms
nmake /f ms\ntdll.mak clean
nmake /f ms\ntdll.mak
nmake /f ms\ntdll.mak install
cd ..
