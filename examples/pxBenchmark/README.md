
 Build Instructions for pxBenchmark

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
    sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake quilt
    
    ~~~~
    
    ~~~~

    sudo apt-get install libncurses5-dev

   ~~~~

## macOS Setup 

>Install Xcode, CMake and quilt
>   * Download the latest version of Xcode from https://developer.apple.com/xcode/download/
>   * Download and install the latest version of brew from https://brew.sh/
>   * From terminal install dependencies: cmake, pkg-config, quilt, java.

```bash
    brew install cmake pkg-config quilt caskroom/cask/java
```

## Windows Setup
>Setup Windows 10
>   * Windows 10 
>   * Visual Studio 2017 community with `Desktop development with C++` workload
>   * [windows sdk 10.0.16299.0](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk),it is included in VS2017 with above workload and only necessary if you have issue to install with VS2017)
>   * python 2.7.x , make sure python can work in cmd (setup environment variables depending on install location)
>   * git for windows , make sure git can work in cmd (setup environment variables depending on install location)
>   * patch utility for windows (this comes with git. setup environment variables depending on install location of patch.exe)
>   * Download and install cmake for windows from https://cmake.org/download/
>   * Download and install NSIS installer from http://nsis.sourceforge.net/Download
>   * **Run all these commands from a Visual Studio Command Prompt**


## Building with CMake (this will work for all platforms after setup is complete)
1. Get source code:

    ~~~~
    git clone https://github.com/ashwinikalokhe/pxCore.git
    ~~~~
  
2. Checkout current development branch

    ~~~~
    cd pxCore
    git checkout pxBenchmark
    ~~~~
     
3. Build **externals**:
    a. Build all externals for use during the pxscene build.
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
  
    b. To use system libraries for external libs during pxscene build, install libs on the system. To build just node, duktape and breakpad with the patches necessary for pxscene, do the following.
      
      For Mac and Linux OS.

      Build duktape
      ~~~~ 
      cd examples/pxScene2d/external/dukluv/
      mkdir build
      cd build
      cmake ..
      cmake --build . --config Release
      ~~~~
      Build Node
      ~~~~
      cd examples/pxScene2d/external/node
      ./configure --shared
      make -j1
      ln -sf out/Release/obj.target/libnode.so.48 libnode.so.48
      ln -sf libnode.so.48 libnode.so
      ln -sf out/Release/libnode.48.dylib libnode.48.dylib
      ln -sf libnode.48.dylib libnode.dylib
      ~~~~
      Build breakpad
      ~~~~
      cd examples/pxScene2d/external/breakpad-chrome_55
      ./configure
      make      
      ~~~~	

      For Windows
      
      Build Duktape
      ~~~~
      cd examples/pxScene2d/external/dukluv/
      patch -p1 -i patches/dukluv.git.patch
      mkdir build
      cd build
      cmake ..
      cmake --build . --config Release -- /m
      ~~~~
      Build node
      ~~~~
      cd examples/pxScene2d/external/libnode-v6.9.0
      CALL vcbuild.bat x86 nosign
      cd ..
      ~~~~
      Build breakpad
      ~~~~	
      cd examples/pxScene2d/external/breakpad-chrome_55
      CALL gyp\gyp.bat src\client\windows\breakpad_client.gyp --no-circular-check
      cd src\client\windows
      msbuild breakpad_client.sln /p:Configuration=Release /p:Platform=Win32 /m
      ~~~~
      
4. Build **pxBenchmark**

    On following step 3b, Specify -DPREFER_SYSTEM_LIBRARIES=ON to use system libraries rather than libraries from externals directory.
    Note :  If a dependent library is not found installed on the system, then the version in externals will be used.
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
    cd pxCore/examples/pxBenchmark/src
    ./pxbenchmark.sh window_width windowHeight unit_widht unit_height
    ~~~~

    On macOS
    ~~~~
    cd pxCore/examples/pxScene2d/src/pxbenchmark.app/Contents/MacOS
    ./pxscene.sh window_width windowHeight unit_widht unit_height
    ~~~~

    On Windows
    ~~~~
    cd pxCore\temp\
    cpack .
    cd pxCore\temp\_CPack_Packages\win32\NSIS\pxbenchmark-setup
    pxbenchmark.exe window_width windowHeight unit_widht unit_height
    ~~~~

Examples:
  ~~~~
./pxbenchmark.sh 1028 720 572 572
./pxbenchmark.sh 1920 1080 50 50 
  ~~~~
Running ./pxbenchmark.sh without a parameter will load application with default window size and unit size

6. Write your own app!

7. Run the unit tests (if they were built using the configuration in step 4)  TODO
   
    ~~~~
    cd pxCore/tests/pxBenchmark
    ~~~~

    On Linux, Mac, and Raspberry Pi run: 
    ~~~~
    ./pxbenchmarktests.sh
    ~~~~

    On Windows
    ~~~~
    pxbenchmarktests.exe
    ~~~~

## Building and running unit tests TODO
1. Get source code
   ~~~~
   git clone https://github.com/ashwinikalokhe/pxCore.git
   ~~~~

2. Build
   ~~~~
   cd pxCore/
   mkdir temp
   cd temp
   cmake -DBUILD_PX_TESTS=ON -DBUILD_PXBENCHMARK_STATIC_LIB=ON -DPXBENCHMARK_TEST_HTTP_CACHE=OFF ..
   cmake --build . --config Release
   ~~~~

3. Run
   ~~~~
   cd pxCore/tests/pxBenchmark
   ./pxbenchmarktests.sh
   ~~~~

## Developer CMake options
   ENABLE_THREAD_SANITIZER - Turn on this option to enable thread sanitizer support.  The default value is OFF

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
   git clone https://github.com/ashwinikalokhe/pxCore.git
   ~~~~
  
2. Checkout current development branch
   ~~~~
   cd pxCore
   git checkout pxBenchmark
   ~~~~
  
3. Build **pxBenchmark** using Xcode
      1. Open project **_pxCore/examples/pxBenchmark/pxBenchmark.xcworkspace_** in Xcode
      2. Select _"pxBenchmark"_ as the target in the pulldown on the toolbar
      3. From menu, go to Product menu and select _"Build"_ or type  ⌘B




