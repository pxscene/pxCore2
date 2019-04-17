
 Build Instructions for pxBenchmark

## Supported Platforms
>   * macOS, Windows, Linux, and Raspberry Pi

## Minimum requirements
>macOS
>   * OS : Macbook Pro (macOS Sierra >= 10.12)
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

## macOS Setup 

>Install Xcode, CMake and quilt
>   * Download the latest version of Xcode ( >= 9.2 <= 10) from https://developer.apple.com/xcode/download/ (XCode 10+ will currently not work!)
>   * Download and install the latest version of brew from https://brew.sh/
>   * From terminal install dependencies: cmake, pkg-config, quilt, java.

```bash
    brew install cmake pkg-config quilt caskroom/cask/java libuv
```

## Windows Setup
>Setup Windows 10
>   * Windows 10 
>   * Visual Studio 2017 community with `Desktop development with C++` workload
>   * [windows sdk 10.0.15063.0, windows sdk 10.0.16299 and windows sdk 10.0.17134(aka 1803)] (https://developer.microsoft.com/en-us/windows/downloads/sdk-archive)
>   * python 2.7.x , make sure python can work in cmd (setup environment variables depending on install location)
>   * git for windows , make sure git can work in cmd (setup environment variables depending on install location). While installing git, make sure that **"Use Git and optional Unix tools from the Windows Command Prompt"** option is checked.
>   * patch utility for windows (this comes with git. setup environment variables depending on install location of patch.exe)
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

3. Build **pxBenchmark**

    On following step 3b, Specify -DPREFER_SYSTEM_LIBRARIES=ON to use system libraries rather than libraries from externals directory.
    Note :  If a dependent library is not found installed on the system, then the version in externals will be used.
    ~~~~
    cd pxCore/
    mkdir temp
    cd temp
    cmake DPXSCENE_COMPILE_WARNINGS_AS_ERRORS=OFF -DPXCORE_COMPILE_WARNINGS_AS_ERRORS=OFF BUILD_STATIC_LIBS=ON  CMAKE_BUILD_TYPE=Release ..  -G Xcode
    ~~~~
    If you wish to build the unit tests then run
    ~~~~
    cmake -DBUILD_PX_TESTS=ON -DBUILD_PXSCENE_STATIC_LIB=ON -DPXSCENE_TEST_HTTP_CACHE=OFF ..
    ~~~~
    For Linux, Mac, and Raspberry Pi run: 
    ~~~~
    cmake --build . --config Release
    ~~~~
    For Windows (**Run from inside a Visual Studio Command Prompt**):
    ~~~~
    cmake --build . --config Release -- /m
    ~~~~

4. Run a sample javascript file:

    On Linux
    ~~~~
    cd pxCore/examples/pxBenchmark/src
    ./pxBenchmark.sh window_width windowHeight unit_widht unit_height
    ~~~~

    On macOS
    ~~~~
    cd pxCore/examples/pxBenchmark/src/pxBenchmark.app/Contents/MacOS
    ./pxBenchmark.sh window_width windowHeight unit_widht unit_height
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

5. Using Automation script to publish results on confluence page
~~~~
    1. Confirm version.txt in current/root folder
    2. Add /opt/pxbenchmark.conf with user:User_Name password:User_Password
    3. Run ./automation.sh 
~~~~
    
5. Run the unit tests (if they were built using the configuration in step 4)
   
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
   cd pxCore/tests/pxBenchmark
   ./pxBenchmarktests.sh
   ~~~~

## Developer CMake options
   ENABLE_THREAD_SANITIZER - Turn on this option to enable thread sanitizer support.  The default value is OFF


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
  
3. Build **pxBenchmark** using Xcode
      1. Open project **_pxCore/examples/pxBenchmark/pxBenchmark.xcworkspace_** in Xcode
      2. Select _"pxBenchmark"_ as the target in the pulldown on the toolbar
      3. From menu, go to Product menu and select _"Build"_ or type  ⌘B




