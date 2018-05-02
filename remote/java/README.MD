# Hercules RT Remote - Java

### Platform and tools

- ubuntu 16.04

### Build remote

* Install dependencies,  `sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake gnutls-bin libgnutls-dev autoconf libtool nasm uuid-dev maven quilt`
* Install java
   * `sudo add-apt-repository ppa:openjdk-r/ppa && sudo apt-get update`
   * `sudo apt-get install openjdk-8-jdk`
* Build dependencies,  `cd examples/pxScene2d/external` and then run `./build.sh`
* Build pxScene
  * `cd ../../..`
  * `mkdir build && cd build`
  * `cmake .. -DSUPPORT_DUKTAPE=OFF && make -j1`
* Build rtRemote, `cd ../remote && make -j1` ,
* Build server example:
  * `cd ../rtRemoteSimpleServer`
  * `mkdir build && cd build`
  * `cmake .. && make`
  * **copy config file to build** **folder** `cp ../rtremote.conf ./`
* Run the sample server `./sample_server`



### Build java remote

- cd `pxCore/rtRemote/java` , `mvn compile`



### Validation

#### examples validation

* startup server
  * **c++ sample server** , Follow *Build remote and run example* section to start up sample server
  * **java sample server**, Follow *Build java remote* to build java codes, then ``cd pxCore/rtRemote/java` use `mvn exec:java -Dexec.mainClass="examples.server.SampleServer"` to startup server.
* Basic client examples , cd `pxCore/rtRemote/java` , `mvn compile`
  * `mvn exec:java -Dexec.mainClass="examples.TypeTest"`, this example include all types test.
  * `mvn exec:java -Dexec.mainClass="examples.MethodTest"`, this example include a lot of method test.
  * `mvn exec:java -Dexec.mainClass="examples.ThreadTest"`, this example create 4 threads for each 10 seconds to test type and methods.


#### loop to find object validation

Loop to find object test , close server , then start `mvn exec:java -Dexec.mainClass="examples.TypeTest"`, you can see client is looping to find remote object, then start server, the client will start working.

#### IntelliJ project validation

- download and install IntelliJ idea https://www.jetbrains.com/idea/
- open intelliJ idea, click open project, then choose folder *pxCore/rtRemote/java*, it should auto load the whole project to ide.



### RTValue types list

| C++ RTRemote type | Java Types |
| ----------------- | ---------- |
| bool              | Boolean    |
| float             | Float      |
| double            | Double     |
| int8_t            | Short      |
| uint8_t           | Short      |
| int32_t           | Integer    |
| uint32_t          | Long       |
| int64_t           | Long       |
| uint64_t          | BigInteger |
| rtString          | String     |
| rtObject          | RTObject   |
| rtFunctionRef     | RTFunction |
| voidPtr           | Long       |
