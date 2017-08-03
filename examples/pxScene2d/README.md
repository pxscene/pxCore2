#Build Instructions for pxScene

## Ubuntu Setup
>Setup Ubuntu
>   * Create a linux VM.  The following version is recommended:  ubuntu-14.04.1-desktop-amd64
   
1. Install required packages:
    
    ~~~~
    sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake x11proto-input-dev
    
    Execute below command:
    
    cd /usr/include/X11/extensions && sudo ln -s XI.h XInput.h
    
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
>   * [windows sdk 10.0.15063.0](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk),it is included in VS2017 with above workload and only necessary if you have issue to install with VS2017)
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
7. Write your own app!




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




