# Description

pxScene / Spark is a wrapper around the pxCore libraries that can run as a stand-along application on OS X, Windows, and Linux.  The wrapper provides Javascript bindings to allow for easy consumption of the low-level pxCore functionality.

Initially, and in the production builds, pxScene / spark supports Node as the Javascript engine for the application.  Node provides lots of nice features that are useful for pxScene development, but it has quite a bit of overhead, so we've been asked to investigate replacing the Node Javascript engine in pxScene with other Javascript engines.

The first implementation we did involved stripping out Node and replacing it with Duktape (http://duktape.org/).  Duktape worked well and the memory overhead was reduced significantly.  The problem with duktape is that it is a little *too* small.  It lacks a lot of nice features that Node currently supports and that are used in the existing pxScene examples, including some ES6 features like:

* `let`
* `const`
* `arrow functions`
* promises
* proxy object support

We investigated adding support for these via a couple of different paths.  Our members had trouble with modifying the duktape parsing path because it's complex, making it hard to add new functionality.  I also got in touch with the main duktape developer and suggested to the client that maybe we contract with him directly to prioritise features, but that didn't really go too far.

# V8

More recently, the client has asked us to investigate supporting the Chrome JS engine called V8 (https://developers.google.com/v8/) as a replacement for Node.  I've worked primarily with Alexey Kuts (kruntuid) to implement the V8 build, since he did a lot of work on the duktape build.  He presented a stepped plan that we approved and worked on directly.

To date, we've been able to build the V8 integration pretty much completely, to the point where it runs the pxScene test suite the same as the Node version, with memory reduction.  The memory reduction isn't as much as we'd probably like so we've been looking at turning off specific features in the V8 build to see if that will help.  We've identified some items of reduction, but nothing overly significant.

# Challenges

One challenge we've had is supporting both Node and V8 in the same executable.  This was possible in the duktape integration - the executable could use either interpreter and would default to one or the other based on files in the user's home directory.

This is not currently possible with the V8 build because Node includes an old version of V8 and the linking of both the old version and the current version causes linker errors.

# Build

Here's instructions for building pxScene with V8 instead of Node.  The biggest change is the `-DSUPPORT_V8=ON` flag in the first call to `cmake`

### Setup for V8 specific build on Linux

* `sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake quilt gnutls-bin libgnutls-dev libuv-dev`
  * On Ubuntu 14.04, for building V8:
  * `sudo add-apt-repository ppa:ubuntu-toolchain-r/test`
  * `sudo apt-get update`
  * `sudo apt-get install libstdc++6`
* `git clone git@github.com:topcoderinc/pxCore.git`
* `cd pxCore`
* `git checkout duktape_proof_of_concept`
* `cd examples/pxScene2d/external`
* `chmod +x buildV8.sh`
* `./buildV8.sh`
* `./build.sh`
* `cd ../../../`
* `mkdir temp`
* `cd temp`
* `cmake -DSUPPORT_V8=ON -DSUPPORT_NODE=OFF ..`
* `cmake --build . --config Release -- -j1`
* `cd ../examples/pxScene2d/src`
* `./Spark about.js`

At this point the `engine` field in about.js should say `V8`.
