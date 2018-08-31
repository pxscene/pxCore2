### Setup for V8 specific build

* `sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake gnutls-bin libgnutls-dev autoconf libtool nasm`
* `git clone git@github.com:topcoderinc/pxCore.git`
* `cd pxCore`
* `git checkout duktape_proof_of_concept`
* `cd examples/pxScene2d/external`
* `./build.sh`
* `cd ../../../`
* `mkdir temp`
* `cd temp`
* `cmake -DSUPPORT_V8=ON ..`
* cmake --build . --config Release -- -j1
* cd ../examples/pxScene2d/src
* ./pxscene about.js
* cd
* git clone https://github.com/pxscene/pxscene.git pxscene_examples
* cd pxscene_examples
* git checkout gh-pages
* Examples can be found in examples/px-reference/gallery
