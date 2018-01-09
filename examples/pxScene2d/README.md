# Build Instructions for pxScene

## Supported Platforms
>   * macOS, Windows, Linux, and Raspberry Pi

## Minimum requirements
>macOS
>   * OS : Macbook Pro (macOS Sierra)
>   * RAM Size : 256 MB
>   * Disk space : 24 MB
>   * Processor speed : 1 GHz

>Windows
>   * OS : Windows 10
>   * OS Build : 15063
>   * RAM Size : 128 MB
>   * Disk space : 24 MB
>   * Processor speed : 1 GHz

## Ubuntu Setup
>Setup Ubuntu
>   * Create a linux VM.  The following version is recommended:  ubuntu-14.04.1-desktop-amd64
   
1. Install required packages:
    
    ~~~~
    sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake
    
    ~~~~

## macOS Setup 

>Install Xcode and CMake
>   * Download the latest version of Xcode from https://developer.apple.com/xcode/download/
>   * Download the latest cmake from https://cmake.org/download/
>   * Install cmake and setup the following symbolic links in /usr/local/bin
1. Install cmake and setup the following symbolic links in /usr/local/bin:
    ~~~~
    ln -s /Applications/CMake.app/Contents/bin/ccmake /usr/local/bin/ccmake 
    ln -s /Applications/CMake.app/Contents/bin/cmake /usr/local/bin/cmake 
    ln -s /Applications/CMake.app/Contents/bin/cmake-gui /usr/local/bin/cmake-gui
    ln -s /Applications/CMake.app/Contents/bin/cpack /usr/local/bin/cpack
    ln -s /Applications/CMake.app/Contents/bin/ctest /usr/local/bin/ctest
    ~~~~



## Windows Setup
>Setup Windows 10
>   * Windows 10 
>   * Visual Studio 2017 community with `Desktop development with C++` workload
>   * [windows sdk 10.0.16299.0](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk),it is included in VS2017 with above workload and only necessary if you have issue to install with VS2017)
>   * python 2.7.x , make sure python can work in cmd (setup environment variables depending on install location)
>   * git for windows , make sure git can work in cmd (setup environment variables depending on install location)
>   * Download and install cmake for windows from https://cmake.org/download/
>   * Download and install NSIS installer from http://nsis.sourceforge.net/Download
>   * **Run all these commands from a Visual Studio Command Prompt**


## Building with CMake (this will work for all platforms after setup is complete)
1. Get source code:

    ~~~~
    git clone https://github.com/pxscene/pxCore.git
    ~~~~
  
2. Checkout current development branch

    ~~~~
    cd pxCore
    git checkout master
    ~~~~

3. Build **externals**:
    ~~~~
    cd examples/pxScene2d/external
    ~~~~
    For Linux and Mac run:
    ~~~~
    ./build.sh
    ~~~~
    For Raspberry Pi run:
    ~~~~
    ./build_rpi.sh
    ~~~~
    For Windows (**Run from inside a Visual Studio Command Prompt**):
    ~~~~
    buildWindows.bat
    ~~~~

4. Build **pxScene**

    ~~~~
    cd pxCore/
    mkdir temp
    cd temp
    cmake ..
    ~~~~
    If you wish to build the unit tests then run
    ~~~~
    cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON -DPXSCENE_TEST_HTTP_CACHE=OFF ..
    ~~~~
    For Linux, Mac, and Raspberry Pi run: 
    ~~~~
    cmake --build . --config Release -- -j1
    ~~~~
    For Windows (**Run from inside a Visual Studio Command Prompt**):
    ~~~~
    cmake --build . --config Release -- /m
    ~~~~

5. Run a sample javascript file:

    On Linux
    ~~~~
    cd pxCore/examples/pxScene2d/src
    ./pxscene.sh {path_to_javascript_file_name}.js
    ~~~~

    On macOS
    ~~~~
    cd pxCore/examples/pxScene2d/src/pxscene.app/Contents/MacOS
    ./pxscene.sh {path_to_javascript_file_name}.js
    ~~~~

    On Windows
    ~~~~
    cd pxCore\temp\
    cpack .
    cd pxCore\temp\_CPack_Packages\win32\NSIS\pxscene-setup
    pxscene.exe {path_to_javascript_file_name}.js
    ~~~~

Examples:
  ~~~~
./pxscene.sh http://www.pxscene.org/examples/px-reference/gallery/picturepile.js
./pxscene.sh http://www.pxscene.org/examples/px-reference/gallery/gallery.js
  ~~~~
Running ./pxscene.sh without a parameter will load the local browser.js that will take a .js pathname relative to http://www.pxscene.org/examples/px-reference/gallery to run.  Alternatively, a fully qualified url can be used, for example:
  ~~~~
http://www.pxscene.org/examples/px-reference/gallery/picturepile.js
http://www.pxscene.org/examples/px-reference/gallery/gallery.js
file:///home/username/directory/filename.js
  ~~~~
6. Write your own app!

7. Run the unit tests (if they were built using the configuration in step 4)
   
    ~~~~
    cd pxCore/tests/pxScene2d
    ~~~~

    On Linux, Mac, and Raspberry Pi run: 
    ~~~~
    ./pxscene2dtests.sh
    ~~~~

    On Windows
    ~~~~
    pxscene2dtests.exe
    ~~~~

## Building and running unit tests
1. Get source code
   ~~~~
   git clone https://github.com/pxscene/pxCore
   ~~~~

2. Build
   ~~~~
   cd pxCore/
   mkdir temp
   cd temp
   cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON -DPXSCENE_TEST_HTTP_CACHE=OFF ..
   cmake --build . --config Release
   ~~~~

3. Run
   ~~~~
   cd pxCore/tests/pxScene2d
   ./pxscene2dtests.sh
   ~~~~


## Building with rtRemote support (Linux only)
1. Get source code
   ~~~~
   git clone https://github.com/pxscene/pxCore
   ~~~~

2. Build
   ~~~~
   cd pxCore/remote
   make
   cd ..
   mkdir temp
   cd temp
   cmake -DBUILD_RTREMOTE_LIBS=ON ..
   cmake --build . --config Release
   ~~~~

   The rtRemote libs will be located in pxCore/remote

   Additional build configurations for rtRemote are:
   ~~~~
   Build rpcSampleApp: -DBUILD_RTREMOTE_SAMPLE_APP_SHARED=ON
   Build rpcSampleApp_s: -DBUILD_RTREMOTE_SAMPLE_APP_STATIC=ON
   Build rtSampleClient and rtSampleServer: -DBUILD_RTREMOTE_SAMPLE_APP_SIMPLE=ON
   Enable rtRemote debugging: -DENABLE_RTREMOTE_DEBUG=ON
   Enable rtRemote profiling: -DENABLE_RTREMOTE_PROFILE=ON
   Disable librtremote shared library building: -DBUILD_RTREMOTE_SHARED_LIB=OFF
   Disable librtremote static library building: -DBUILD_RTREMOTE_STATIC_LIB=OFF
   Output pxCore libs locallay: -DOUTPUT_LIBS_LOCAL=ON
   Disable building pxCore libs: -DBUILD_PXCORE_LIBS=OFF
   Disable building pxscene: -DBUILD_PXSCENE=OFF
   ~~~~

## On Mac OS X - Xcode Workspace 

1. Get source code
  
   ~~~~
   git clone https://github.com/pxscene/pxCore.git
   ~~~~
  
2. Checkout current development branch
   ~~~~
   cd pxCore
   git checkout master
   ~~~~
  
3. Build **pxScene2d** using Xcode
      1. Open project **_pxCore/examples/pxScene2d/pxScene2d.xcworkspace_** in Xcode
      2. Select _"pxScene2d"_ as the target in the pulldown on the toolbar
      3. From menu, go to Product menu and select _"Build"_ or type  ⌘B




