==================================
build instructions for windows 10:
==================================

set things up:
https://github.com/pxscene/pxCore/tree/master/examples/pxScene2d#windows-setup

these commands must be executed in msvc 2017 developer prompt:

# building pxScene (https://github.com/pxscene/pxCore/tree/master/examples/pxScene2d)

cd ..\..\..
cd examples/pxScene2d/external
buildWindows.bat

# NOTE: in case of an libnode configure error rerun buildWindows.bat

cd ..\..\..
mkdir temp
cd temp
cmake ..
cmake --build . --config Release -- /m

# install
cpack .
cd _CPack_Packages\win32\NSIS\pxscene-setup

# test
pxscene.exe browser.js
pxscene.exe about.js

# in editbox you can enter about.js (served from local disk)
# or fonts2.js (served from http://www.pxscene.org/examples/px-reference/gallery/)

# to test examples locally
# copy <somewhere>/px-reference .
# pxscene.exe px-reference\gallery\fonts2.js

If you build with enabled node and duktape (they both are built by default)
and file with name %USERPROFILE%\.sparkUseDuktape exists, 
duktape engine is used, otherwise node is used


================================================================
build instructions for ubuntu (tested on 16.04, 14.04 is ok too)
================================================================

set things up:
https://github.com/pxscene/pxCore/tree/master/examples/pxScene2d#ubuntu-setup
('Create a linux VM' is optional there)

git clone -b duktape_proof_of_concept https://github.com/topcoderinc/pxCore.git
cd pxCore

for ubuntu x64, you may need 
cd zlib
sudo make install
cd ../png
./configure --prefix=/usr LDFLAGS="-L/usr/lib -lz"
make clean && make

cd ../../../
cd examples/pxScene2d/external
./build.sh

cd ../../../
mkdir temp
cd temp
cmake ..
cmake --build . --config Release -- -j1

cpack .
cd _CPack_Packages/Linux/TZ/pxscene-setup/
./pxscene browser.js
./pxscene about.js

If you build with enabled node and duktape (they both are built by default)
and file with name $HOME/.sparkUseDuktape exists, 
duktape engine is used, otherwise node is used

================================================================
build instructions for macosx
================================================================

set things up:
https://github.com/krunt/pxCore/tree/_duktape/examples/pxScene2d#macos-setup

git clone -b duktape_proof_of_concept https://github.com/topcoderinc/pxCore.git
cd pxCore

cd examples/pxScene2d/external
./build.sh

cd ../../../
mkdir temp
cd temp
cmake ..
cmake --build . --config Release -- -j1

cd ../examples/pxScene2d/src/pxscene.app/Contents/MacOS
./pxscene.sh browser.js
./pxscene.sh about.js

If you build with enabled node and duktape (they both are built by default)
and file with name $HOME/.sparkUseDuktape exists, 
duktape engine is used, otherwise node is used
