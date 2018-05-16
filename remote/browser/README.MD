# Hercules RT Remote - Browser

### Platform and tools

- c++ rtRemote server on ubuntu 16.04
- client run in browser, any platform.

### Download pxCore code

-  `git clone https://github.com/topcoderinc/pxCore.git`
-  `cd pxCore`
-  `git checkout 8570e296245b6f675acfc79dd45329cc1ccab9d2`
-  `git apply remote-browser.patch`

### Build remote and server

* Install dependencies,  `sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake gnutls-bin libgnutls-dev autoconf libtool nasm uuid-dev maven quilt`
* Install node, the ubuntu 16.04 installed npm 3.x and node 4.x, but we need 8.9.x
   * `sudo npm i -g n`
   * `sudo n 8.9.4`
   * `sudo ln -sf /usr/local/n/versions/node/8.9.4/bin/node /usr/bin/node`
   * `sudo ln -sf /usr/local/n/versions/node/8.9.4/bin/npm /usr/bin/npm`
* Build dependencies,  `cd examples/pxScene2d/external` and then run `./build_remote.sh`
* Build pxScene
  * `cd ../../..`
  * `mkdir build && cd build`
  * `cmake .. -DSUPPORT_DUKTAPE=OFF && make -j1`
* Build rtRemote,
   * `mkdir ../remote/build `
   * `cd ../remote/build `
   * `cmake .. && make -j2`
* Run the sample server `cd ..`, `./rtSampleServer`

### Build remote lib and run examples

#### Build remote browser lib

- goto remote browser lib folder, `cd pxCore/remote/browser/remote-lib`
- run  `npm i` to install all dependency, and run `npm run lint` to check code
- run `npm run build` to build *dist/rt-remote.min.js*, use `cp dist/rt-remote.min.js ../browser-examples/` copy dist lib js file to examples *browser-examples*

note:   for browser, just need use this include to html , then remote lib all modules will include in RTRemote namespace, you can sample use like this *RTRemote.RTRemoteConnectionManager*

```html
 <script src="rt-remote.min.js"></script>
```

#### Run examples

- got remote examples folder, ``cd pxCore/remote/browser/browser-examples`
- use `sudo npm -g install bower http-server ` install common tools.
- use `bower install` install bootstrap and other third lib.
- **open  *js/app.js* , update *websocketURI* with your rtRemote Server computer ip.**
- then use `http-server -p 3000` to serve examples.

### Validation

#### examples validation

* startup c++ server
  * **c++ sample server** , Follow *Build remote and server* section to start up sample server

* run basic node client examples to make sure the RtRemote c++ worked with old examples, cd `pxCore/remote/js` ,and run `npm i` to install dependencies, then run
  * ` node examples/typeTest.js`
  * `node examples/methodTest.js`

  * `node examples/multiObjTest.js`

* browser examples, please start up c++ server first, and follow *Build remote lib and run examples*  to start up examples server.

  * update *js/app.js* websocketURI with your rt remote server computer ip.
  * use chrome open http://<your examples server ip>:3000
  * after example initialized, click *Run All Examples* to test.

  notes: i tested chrome,safari,firefox.

## Notes

- because of master branch missing the new rtRemote c++ featrues, so i had to merge this code first https://github.com/pxscene/pxCore/pull/1012
