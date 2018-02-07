# Hercules RT Remote - Java implementation

### Platform and tools

- ubuntu 16.04

### Download pxCore code

-  `git clone https://github.com/gladish/pxCore.git`
- `cd pxCore`
- `git checkout da1d375f52e3e0399548d8d574aa60b003651e2e`



### Build remote and run example

* Install dependencies,  `sudo apt-get install git libglew-dev freeglut3 freeglut3-dev libgcrypt11-dev zlib1g-dev g++ libssl-dev nasm autoconf libtool cmake gnutls-bin libgnutls-dev autoconf libtool nasm uuid-dev maven`
* Install java
 * `sudo add-apt-repository ppa:openjdk-r/ppa && sudo apt-get update`
 * `sudo apt-get install openjdk-8-jdk`

* Build pxScene according to the documentation in step-by-step-duktape-linux-build.md
* Build rtRemote, `cd pxCore/remote && make -j1` ,
* Build server example:
  * `cd pxCore/rtRemoteSimpleServer`
  * `mkdir build && cd build`
  * `cmake .. && make`
* Run the sample server `./sample_server`

### Build java remote client

- cd `pxCore/rtRemote/java` , `mvn compile`

### Validation

* Follow *Build remote and run example* section to start up sample server
* Basic client examples , cd `pxCore/rtRemote/java` , `mvn compile`
  * `mvn exec:java -Dexec.mainClass="examples.TypeTest"`
  * `mvn exec:java -Dexec.mainClass="examples.MethodTest"`
  * `mvn exec:java -Dexec.mainClass="examples.ThreadTest"`
* Loop to find object test , close server , then start `mvn exec:java -Dexec.mainClass="examples.TypeTest"`, you can see client is loop find remote object, then start server, client will be start work.


### RTValue types list

| C++ RTRemote type | Java Types    |
| ----------------- | ------------- |
| bool              | boolean       |
| float             | Float         |
| double            | Double        |
| int8_t            | Short         |
| uint8_t           | Integer       |
| int32_t           | Integer       |
| uint32_t          | Long          |
| int64_t           | Long          |
| uint64_t          | BigInteger    |
| rtString          | String        |
| rtObject          | RTLocalObject |
| rtFunctionRef     | RTFunction    |
| voidPtr           | Long          |

## Notes

- c++ rtRemote server didn't suport define **void** type properties, so *TypeTest* ignored set void test

- c++ rtRemote server didn't implement **RT_valueType** type properties, so *TypeTest* ignored set/get **VALUE** test

- all client examples run about long time, like 10-30mins, c++ remote server crashed, maybe c++ remote code had mem leak problem or something else.

  ```
  terminate called after throwing an instance of 'std::bad_alloc'
    what():  std::bad_alloc
  Aborted (core dumped)
  ```

  ​
