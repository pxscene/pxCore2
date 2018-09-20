#!/bin/bash

git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git depot_tools
export PATH=$PWD/depot_tools/:$PATH

gclient

fetch v8
cd v8

gclient sync --no-history --with_tags -r 6.9.351
python tools/dev/v8gen.py x64.release -- is_debug=false v8_enable_i18n_support=false target_cpu=\"x64\" is_component_build=false v8_static_library=true use_custom_libcxx = false use_custom_libcxx_for_host = false

ninja -C out.gn/x64.release v8

python ../binfile2cpp.py out.gn/x64.release/natives_blob.bin out.gn/x64.release/v8_binfile.h out.gn/x64.release/v8_binfile.cpp
python ../binfile2cpp.py out.gn/x64.release/snapshot_blob.bin out.gn/x64.release/v8_binfile.h out.gn/x64.release/v8_binfile.cpp

cd ..
