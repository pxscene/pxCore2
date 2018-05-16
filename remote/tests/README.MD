# Hercules C++ rtRemote - Unit Tests

## Platform and tools
- ubuntu 16.04
- lcov 1.12-14
- gtest

## Download pxCore code
please use https://github.com/topcoderinc/pxCore/tree/_rtRemote_Cpp_tests.
and use commit code '36d1461dda1df422176cd63e402d6115dc296f35'
```
git clone -b _rtRemote_Cpp_tests https://github.com/topcoderinc/pxCore.git
cd pxCore
git checkout 36d1461dda1df422176cd63e402d6115dc296f35
```

## apply patch
copy unitTest.patch to pxCore . Notice this patch file is against commit hash 36d1461dda1df422176cd63e402d6115dc296f35.
```
git apply submission.patch
```

## install lcov
- install lcov
```
git clone https://github.com/linux-test-project/lcov.git
cd lcov
make install
```

or use
```
sudo apt-get update
sudo apt-get install lcov
```

## Build remote and server example

- install dependencies
```
sudo apt-get update
sudo apt-get install git uuid-dev libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake gnutls-bin libgnutls-dev autoconf libtool nasm quilt uuid-dev
```

- build dependencies,  `cd pxCore/examples/pxScene2d/external`  then run `./build.sh`

- build pxcore and pxscene
```
cd pxCore
mkdir temp
cd temp
cmake .. -DSUPPORT_DUKTAPE=OFF
cmake --build . --config Release -- -j1
```

- build rtRemote
```
cd pxCore/remote
make COVERAGE=1
```
Note: build rtRemote should use **make COVERAGE=1** command not **make** command.**make COVERAGE=1** will generate code coverage related files in obj.

- build server example
```
cd pxCore/rtRemoteSimpleServer
mkdir build
cd build
cmake ..
make
```
You will get generated executable program **sample_server**.

## build tests

```
cd pxCore/remote/tests
mkdir build
cd build
cmake ..
make
```
Then you will get generated 3 executable programs
- **rtUnitTest**: Unit test in client mode
- **rtUnitTestServer**: Unit test in server mode
- **client_for_server_test**: The client for unit testing by rtUnitTestServer

## Validation
please see Validation.MD
