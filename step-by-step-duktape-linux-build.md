### Setup

* sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake gnutls-bin libgnutls-dev autoconf libtool nasm
* git clone git@github.com:topcoderinc/pxCore.git
* cd pxCore
* git checkout duktape_proof_of_concept
* cd src/dukluv
* mkdir build
* cd build
* cmake ..
* make
* cd ../../../
* cd examples/pxScene2d/external
* cd curl
* ./configure --with-gnutls
* make all
* cd ..
* cd png
* ./configure
* make all
* cd ..
* cd ft
* export LIBPNG_LIBS="-L../png/.libs -lpng16"
* ./configure --with-png=no
* make all
* cd ..
* cd jpg
* ./configure
* make all
* cd ..
* cd zlib
* ./configure
* make all
* cd ..
* cd libjpeg-turbo
* git update-index --assume-unchanged Makefile.in
* git update-index --assume-unchanged aclocal.m4
* git update-index --assume-unchanged ar-lib
* git update-index --assume-unchanged compile
* git update-index --assume-unchanged config.guess
* git update-index --assume-unchanged config.h.in
* git update-index --assume-unchanged config.sub
* git update-index --assume-unchanged configure
* git update-index --assume-unchanged depcomp
* git update-index --assume-unchanged install-sh
* git update-index --assume-unchanged java/Makefile.in
* git update-index --assume-unchanged ltmain.sh
* git update-index --assume-unchanged md5/Makefile.in
* git update-index --assume-unchanged missing
* git update-index --assume-unchanged simd/Makefile.in
* autoreconf -f -i
* ./configure
* make
* cd ..
* cd breakpad
* ./configure
* make
* cd ..
* cd ../../../
* mkdir temp
* cd temp
* cmake ..
* cmake --build . --config Release -- -j1
* cd ../examples/pxScene2d/src
* ./pxscene about.js
* cd
* git clone https://github.com/pxscene/pxscene.git pxscene_examples
* cd pxscene_examples
* git checkout gh-pages
* Examples can be found in examples/px-reference/gallery
