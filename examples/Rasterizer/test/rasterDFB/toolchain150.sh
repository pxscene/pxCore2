#!/bin/bash

# component-specific vars
export RDK_PROJECT_ROOT_PATH=/home/hfitzpatrick/projects/RNG150/master_HEAD
export RDK_FSROOT_PATH=${RDK_PROJECT_ROOT_PATH}/sdk/fsroot/ramdisk
export FSROOT=${RDK_FSROOT_PATH}
export TOOLCHAIN_DIR=${RDK_TOOLCHAIN_PATH}
export WORK_DIR=${RDK_PROJECT_ROOT_PATH}/workRNG150
export SDK_INCLUDE=${RDK_PROJECT_ROOT_PATH}/sdk/fsroot/ramdisk_bck_packaged/usr/local/include
source $WORK_DIR/../sdk/scripts/setBCMenv.sh

export CROSS_COMPILE=mipsel-linux-
export GCC=${CROSS_COMPILE}gcc
export GXX=${CROSS_COMPILE}g++
export LD=${CROSS_COMPILE}ld
export CC=${CROSS_COMPILE}gcc
export CXX=${CROSS_COMPILE}g++
export STRIP=mipsel-linux-uclibc-strip

