
mkdir build
cd build
cmake ..

# TODO:  Turn off surplus stuff... below is incomplete.
#
# cmake -DWEBP_ENABLE_SIMD=ON\
#       -DWEBP_BUILD_ANIM_UTILS=OFF\
#       -DWEBP_BUILD_CWEBP=OFF\
#       -DWEBP_BUILD_DWEBP=OFF\
#       -DWEBP_BUILD_GIF2WEBP=OFF\
#       -DWEBP_BUILD_IMG2WEBP=OFF\
#       -DWEBP_BUILD_VWEBP=OFF\
#       -DWEBP_BUILD_WEBPINFO=OFF\
#       -DWEBP_BUILD_WEBPMUX=OFF\
#       -DWEBP_BUILD_EXTRAS=OFF\
#       -DWEBP_BUILD_WEBP_JS=OFF\
#       -DWEBP_NEAR_LOSSLESS=OFF ..

make

EXT_INSTALL_PATH=$PWD/../../artifacts/${TRAVIS_OS_NAME}

# library

cp libwebp.a $EXT_INSTALL_PATH/lib

# includes
cp -R ../src/webp $EXT_INSTALL_PATH/include/