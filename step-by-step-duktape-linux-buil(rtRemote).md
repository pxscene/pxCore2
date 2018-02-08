### Setup

#### Ubuntu dependencies
* sudo apt-get install git uuid-dev libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake gnutls-bin libgnutls-dev autoconf libtool nasm quilt uuid-dev maven

#### Get the repo set up
* `git clone git@github.com:topcoderinc/pxCore.git`
* `cd pxCore`
* `git checkout _rtRemote_Java`

#### libcurl
* `cd examples/pxScene2d/external`
* `cd curl`
* `./configure --with-gnutls`
* `make all`

#### libpng
* `cd ..`
* `cd png`
* `./configure`
* `make all`

#### freetype
* `cd ..`
* `cd ft`
* `export LIBPNG_LIBS="-L../png/.libs -lpng16"`
* ``./configure --with-png=no`
* `make all`

#### libjpg
* `cd ..`
* `cd jpg`
* `./configure`
* `make all`

#### zlib
* `cd ..`
* `cd zlib`
* `./configure`
* `make all`

#### libjpeg-turbo
* `cd ..`
* `cd libjpeg-turbo`
* `git update-index --assume-unchanged Makefile.in`
* `git update-index --assume-unchanged aclocal.m4`
* `git update-index --assume-unchanged ar-lib`
* `git update-index --assume-unchanged compile`
* `git update-index --assume-unchanged config.guess`
* `git update-index --assume-unchanged config.h.in`
* `git update-index --assume-unchanged config.sub`
* `git update-index --assume-unchanged configure`
* `git update-index --assume-unchanged depcomp`
* `git update-index --assume-unchanged install-sh`
* `git update-index --assume-unchanged java/Makefile.in`
* `git update-index --assume-unchanged ltmain.sh`
* `git update-index --assume-unchanged md5/Makefile.in`
* `git update-index --assume-unchanged missing`
* `git update-index --assume-unchanged simd/Makefile.in`
* `autoreconf -f -i`
* `./configure`
* `make`

#### breakpad
* `cd ..`
* `cd breakpad`
* `./configure`
* `make`

#### node
* `cd ..`
* `cd node`
* `./configure --shared`
* `make`
* `ln -sf out/Release/obj.target/libnode.so.48 libnode.so.48`
* `ln -sf libnode.so.48 libnode.so`
* `ln -sf out/Release/libnode.48.dylib libnode.48.dylib`
* `ln -sf libnode.48.dylib libnode.dylib`

#### duktape
* `cd ..`
* `cd dukluv`
* `quilt push -aq || test $? = 2`
* `mkdir -p build`
* `cd build`
* `cmake ..`
* `make`

#### pxScene
* `cd ../..`
* `cd ../../../`
* `mkdir temp`
* `cd temp`
* `cmake .. -DSUPPORT_DUKTAPE=OFF`
* `cmake --build . --config Release -- -j1`
* `cd ../examples/pxScene2d/src`
* Validate that pxScene works correctly
  * `./pxscene about.js ``

### rtRemote Java
* Install dependencies,
  * `sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake gnutls-bin libgnutls-dev autoconf libtool nasm uuid-dev maven`
* Install java
 * `sudo add-apt-repository ppa:openjdk-r/ppa && sudo apt-get update`
 * `sudo apt-get install openjdk-8-jdk`
* Build rtRemote, `cd pxCore/remote && make -j1` ,
* Build server example:
  * `cd pxCore/rtRemoteSimpleServer`
  * `mkdir build && cd build`
  * `cmake .. && make`
* Run the sample server `./sample_server`

#### Build java remote client
* cd `pxCore/remote/java` , `mvn compile`

#### Validation

* Follow *Build remote and run example* section to start up sample server
* Basic client examples
  * `mvn exec:java -Dexec.mainClass="examples.TypeTest"`
  * `mvn exec:java -Dexec.mainClass="examples.MethodTest"`
  * `mvn exec:java -Dexec.mainClass="examples.ThreadTest"`
