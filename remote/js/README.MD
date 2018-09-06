# Hercules RT Remote - Node

### Platform and tools

- ubuntu 16.04

### Build remote

* Install dependencies,  `sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake gnutls-bin libgnutls-dev autoconf libtool nasm uuid-dev maven quilt`
* Install java
   * `sudo add-apt-repository ppa:openjdk-r/ppa && sudo apt-get update`
   * `sudo apt-get install openjdk-8-jdk`
* Install node, the ubuntu 16.04 installed npm 3.x and node 4.x, but we need 8.9.x
   * `sudo npm i -g n`
   * `sudo n 8.9.4`
   * `sudo ln -sf /usr/local/n/versions/node/8.9.4/bin/node /usr/bin/node`
   * `sudo ln -sf /usr/local/n/versions/node/8.9.4/bin/npm /usr/bin/npm`
* Build dependencies,  `cd examples/pxScene2d/external` and then run `./build.sh`
* Build pxScene
  * `cd ../../..`
  * `mkdir build && cd build`
  * `cmake .. -DSUPPORT_DUKTAPE=OFF && make -j1`
* Build rtRemote,
   * `mkdir ../remote/build `
   * `cd ../remote/build `
   * `cmake .. && make -j2`
* Build server example:
  * `cd ../../rtRemoteSimpleServer`
  * `mkdir build && cd build`
  * `cmake .. && make`
* Run the sample server `./sample_server`



### Build java remote

- cd `pxCore/remote/java` , `mvn compile`



### Validation

#### examples validation

* startup c++/java/node server
  * **c++ sample server** , Follow *Build remote* section to start up sample server
  * **java sample server**, Follow *Build java remote* to build java codes, then ``cd pxCore/remote/java` use `mvn exec:java -Dexec.mainClass="examples.server.SampleServer"` to startup server.
  * **node sample server,** Follow *Build remote* section to install node, then `cd pxCore/remote/js && npm i ` , and use `node examples/server/sampleServer.js` to start node server.

* basic client examples , cd `pxCore/remote/js` ,and run `npm i` to install dependencies, then run `npm run lint` to check code.
  * ` node examples/typeTest.js`, this example include all types test, each type had at least 2 examples,  the test total had 38 examples, the examples include type *FUNCTION, OBJECT,FLOAT,BOOLEAN,INT8,UINT8,INT32,UINT32,INT64,UINT64,DOUBLE,STRING,VOIDPTR*

  * `node examples/methodTest.js`, this example include 5 methods examples, this include

    method0AndReturn10 -> the method with no params need and return 10 (INT32)
    twoIntNumberSum -> the method need two INT32 params and return this two number sum
    twoFloatNumberSum -> the method need two FLOAT params and return this two number sum
    method1IntAndNoReturn -> the method need a params and no returns
    method2FunctionAndNoReturn -> the method need two params (params 1 is a function, params 2 is INT32), then no returns, but server should invoke the params1 function with args

  * `node examples/multiObjTest.js`, this example create 4 remote objects , and do basic test and method test for each remote object in **parallel** mode.

* basic sample server examples,  **first make sure you start node server**

  * do node client examples (see above section)
  * do java client exmaples cd `pxCore/remote/java` , `mvn compile`
    * `mvn exec:java -Dexec.mainClass="examples.TypeTest"`, this example include all types test.
    *  `mvn exec:java -Dexec.mainClass="examples.MethodTest"`, this example include a lot of method test.
    * `mvn exec:java -Dexec.mainClass="examples.ThreadTest"`, this example create 4 threads for each 10 seconds to test type and methods.


#### loop to find object validation

Loop to find object test , close server , then start `node examples/typeTest.js`, you can see client is looping to find remote object, then start server, the client will start working.

### RTValue types list

note that , **BigNumber** from lib ' **bignumer.js**'

| C++ RTRemote type | Node js Types |
| ----------------- | ------------- |
| bool              | boolean       |
| float             | number        |
| double            | number        |
| int8_t            | number        |
| uint8_t           | number        |
| int32_t           | number        |
| uint32_t          | number        |
| int64_t           | BigNumber     |
| uint64_t          | BigNumber     |
| rtString          | string        |
| rtObject          | object        |
| rtFunctionRef     | function      |
| voidPtr           | BigNumber     |



## Notes

- c++ rtRemote server didn't suport define **void** type properties, so *TypeTest* ignored set void test

- c++ rtRemote server didn't implement **RT_valueType** type properties, so *TypeTest* ignored set/get **VALUE** test

- all client examples run about long time, like 10-30mins, c++ remote server crashed, c++ remote code had mem leak problem or something else ( **java server didn't have these errors**).

  ```
  terminate called after throwing an instance of 'std::bad_alloc'
    what():  std::bad_alloc
  Aborted (core dumped)
  ```
