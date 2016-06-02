1. Install build dependencies. My ubuntu 14 VM already had nodejs installed... 
    I think. Or installing node-gyp will install it.

  sudo  apt-get install node-gyp

2. Build. You only need to run configure when you add/remove source files.

  node-gyp configure
  node-gyp --debug build

  NOTE: for Release builds drop the --debug Then update hello.js
        to reference ./build/Release/px package.

3. Run

  Because of the libraries linked in, you can either set LD_LIBRARY_PATH,
  use the run.sh script.

  export LD_LIBRARY_PATH=../../externa/png/.libs
  nodejs hello.js

  or

  ./run.sh
