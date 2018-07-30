@where msbuild 2> nul
@if %errorlevel% neq 0 (
 @echo.
 @echo Please execute this file from inside Visual Studio's Developer Command Prompt
 @echo.
 pause
 goto :eof
)

cmd /c git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git depot_tools
set PATH=%cd%\depot_tools;%PATH%

cmd.exe /c gclient

cmd /c fetch v8
cd v8

cmd /c gclient sync --no-history --with_tags -r 6.9.351
cmd /c python tools/dev/v8gen.py ia32.release -- is_debug=false v8_enable_i18n_support=false target_cpu=\"x86\" is_component_build=true v8_static_library=true

cmd /c ninja -C out.gn/ia32.release

cmd /c python ..\binfile2cpp.py out.gn\ia32.release\natives_blob.bin out.gn\ia32.release\v8_binfile.h out.gn\ia32.release\v8_binfile.cpp
cmd /c python ..\binfile2cpp.py out.gn\ia32.release\snapshot_blob.bin out.gn\ia32.release\v8_binfile.h out.gn\ia32.release\v8_binfile.cpp

cd ..

