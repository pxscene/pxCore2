#!/bin/bash
set -e
trap 'spd-say -i -20 oops' ERR  #  SAY "OOPS" on ERROR !

# Any subsequent(*) commands which fail will cause the shell script to exit immediately

source toolchain150.sh

#export PXCORE_INCLUDES=" -I${DFB_ROOT}/usr/local/include/directfb -I${WORK_DIR}/Refsw/uclinux-rootfs/lib/zlib/ -I${WORK_DIR}/Refsw/uclinux-rootfs/lib/freetype/include -I${SDK_INCLUDE} -I${SDK_INCLUDE}/png -I${WORK_DIR}/rootfs/usr/local/include -I${WORK_DIR}/rootfs/usr/local/include/png -I${RDK_PROJECT_ROOT_PATH}/pxCore/src/x11 -I${RDK_PROJECT_ROOT_PATH}/pxCore/examples/pxScene2d/external/libnode-v0.12.7/deps/v8/src/ "

#export PXSCENE_LIB_LINKING="-L${WORK_DIR}/rootfs/usr/local/lib"

export DFB_INCLUDES=" -I${DFB_ROOT}/usr/local/include/directfb"
export DFB_LIBS=" -L${DFB_ROOT}/usr/local/lib"

make

printf "\n\n Build completed ...\n\n"
date
printf "\n...Done.\n\n"

spd-say -i -20 ok  #  SAY "OK" on SUCCESS !
