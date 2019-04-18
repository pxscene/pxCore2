
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
4. Turn On pxBenchmark 
    ~~~~    
     set following pxBenchmark flag on in pxCore/CMakelists.txt
     option(BUILD_PXBENCHMARK "BUILD_PXBENCHMARK" ON)
    ~~~~
     
5. Build **pxScene**

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

6. Run a sample javascript file:

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
 
  Running ./pxbenchmark.sh without a parameter will load application with default window size and unit size

 Output data interpretation
 
 At the end of executing pxBenchmark app, output is collected in excel sheet named tmp/pxBenchmark_Output_table.xls.
 This sheet has a table with following structure.
 
 Group    Problem Space    Samples    Iterations    Failure    Baseline    TotalTime(us)    us/Iteration    Iterations/sec    Min (us)    Mean (us)    Max (us)    Variance    Standard Deviation    Skewness    Kurtosis    Z Score
 
 
 Group = Name of pxCore API
 Problem Space    
 Samples    
 Iterations  = current number of iteration. 
 Baseline = 1 as we dont have any as of now
 TotalTime(us) = total time till now.
 us/Iteration  = time per iteration
 Iterations/sec  = number of iterations per seconds 
 Min (us)  = Min time for running specified iterations 
 Mean (us)  = Mean of time required to run specified iteration 
 Max (us)  = Max of time required to run specified iteration 
 Variance
 Standard Deviation    
 Skewness    
 Kurtosis    
 Z Score
 
 There is another table in the bottom of sheet which has following data
 Device Type    Firmware    Date    GPU(ms)    CPU(ms)    NOTES                                            
 Raspberry Pi 3 B+    RPIMC_VBN_default_20181107195904sdy_RAD_3BP    11/19/18    7261    N/A    
 
 Device Type (Type of device in use)    Firmware(Device firmware)    Date (Current date)   GPU(ms) (GPU execution time required to run all pxCore apis with given number of iterations)   CPU(ms) (CPU execution time required to run all pxCore apis with given number of iterations)   NOTES (Total Time required to run all experiments and Frame per second rate FPS with other details) 
 
 Data publishes on a confluence page using automation.sh script
 https://etwiki.sys.comcast.net/pages/viewpage.action?pageId=556737708
  ~~~~
 
  
  
7. Using Automation script to publish results on confluence page
~~~~
    1. Confirm version.txt in current/root folder
    2. Add /opt/pxbenchmark.conf with user:User_Name password:User_Password
    3. Run ./automation.sh 
~~~~
    
8. Run the unit tests (if they were built using the configuration in step 4)
   
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



