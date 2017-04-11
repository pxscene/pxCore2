#Build Instructions for pxScene

## On Ubuntu
>Prerequisite:  Setup Ubuntu
   * Create a linux VM.  The following version is recommended:  ubuntu-14.04.1-desktop-amd64
   
1. Install required packages:
  ~~~~
sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool
  ~~~~

1. Get source code:
  ~~~~
git clone https://github.com/johnrobinsn/pxCore.git
  ~~~~
  
1. Checkout current development branch
  ~~~~
cd pxCore
git checkout master
  ~~~~

1. Build externals:
  ~~~~
cd examples/pxScene2d/external
./build.sh
  ~~~~

1. Build pxCore
  ~~~~
cd ~/pxCore 
make -f Makefile.glut
  ~~~~

1. Build pxscene
  ~~~~
cd ~/pxCore/examples/pxScene2d/src
make -f Makefile
  ~~~~

1. Run a sample javascript file:
  ~~~~
cd ~/pxCore/examples/pxScene2d/src
./pxscene.sh {path_to_javascript_file_name}.js
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
9. Write your own app!


## On Mac OS X 

>Prerequisite:  Install Xcode
>   * Download the latest version of Xcode from https://developer.apple.com/xcode/download/

1. Get source code
  ~~~~
  git clone https://github.com/johnrobinsn/pxCore.git
  ~~~~
  
2. Checkout current development branch
  ~~~~
  cd pxCore
  git checkout master
  ~~~~
  
3. Build externals
  ~~~~
  cd examples/pxScene2d/external
  ./build.sh
  ~~~~
> Note that building libnode-v6.9.0 in this step uses Python, but the build only supports Python 2

4. Build pxCore ...
> Choose step i or ii

  1. Build pxCore using 'xcodebuild' from command line in 'Terminal'
  
    ~~~~
cd ~/pxCore
xcodebuild -scheme "pxCore Static Library" -project pxCore.xcodeproj
    ~~~~
  1. Build pxCore using Xcode
    1. Open project ~/pxCore/pxCore.xcodeproj in Xcode
    2. Select "pxCore Static Library" as the target in the pulldown on the toolbar
    3. From menu, go to Product and select "Build" or type  ⌘B
    
5. Build pxscene
  ~~~~
cd ~/pxCore/examples/pxScene2d/src
make -j
  ~~~~
  
6. Run pxscene: 
  1. Option 1
    1. Open the pxscene application that was just built as ~/pxCore/example/pxScene2d/src/pxscene
    2. In the browser that opens, type in a js file to run.  Path names will be relative to http://www.pxscene.org/examples/px-reference/gallery. Alternatively, a fully qualified url can be used, for example:
  ~~~~
http://www.pxscene.org/examples/px-reference/gallery/picturepile.js
http://www.pxscene.org/examples/px-reference/gallery/gallery.js
file:///home/username/directory/filename.js
  ~~~~
    
  2. Option 2
    1. Use the script within the newly built pxscene app to start pxscene and pass the path to the js file to run:
  ~~~~ 
cd ~/pxCore/examples/pxScene2d/src/pxscene.app/Contents/MacOS
./pxscene.sh {path_to_javascript_file_name}.js
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


## On Raspberry Pi


1. Get source code
  ~~~~
  git clone https://github.com/johnrobinsn/pxCore.git
  ~~~~
  
2. Checkout current development branch
  ~~~~
  cd pxCore
  git checkout master
  ~~~~
  
3. Build externals
  ~~~~
  cd examples/pxScene2d/external
  ./build_rpi.sh
  ~~~~

1. Build pxCore
  ~~~~
cd ~/pxCore/src
make -f Makefile.egl
  ~~~~

1. Build pxscene
  ~~~~
cd ~/pxCore/examples/pxScene2d/src
make
  ~~~~

1. Run a sample javascript file:
  ~~~~
cd ~/pxCore/examples/pxScene2d/src
./pxscene.sh {path_to_javascript_file_name}.js
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
1. Write your own app!


