1. Install build dependencies. My ubuntu 14 VM already had nodejs installed... 
    I think. Or installing node-gyp will install it.

  sudo  apt-get install node-gyp

2. Build. You only need to run configure when you add/remove source files.

  node-gyp configure
  node-gyp build

3. Run

  nodejs hello.js
