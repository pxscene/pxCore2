# Validation

## Unit test in client mode

We run the unit test code as a client, so we need an external **sample_server** as the server side.

### (1) run sample_server

   follow **Build remote and server example** section in README.MD to start up sample server.Then in `pxCore/rtRemoteSimpleServer/build`,
   run `./sample_server`

### (2) run rtUnitTest

Before running rtUnitTest, please build rtUnitTest as **build tests** section in README.MD. And You should make sure you have built rtRemote by command **make COVERAGE=1** .

Open the other terminal, switch to `pxCore/remote/tests/build` folder, run the command below:
```
./rtUnitTest
```

**NOTE** Before running `./rtUnitTest`, you can check if you have permission to run `ifconfig lo down`, if you got this error
```
SIOCSIFFLAGS: Operation not permitted
```

You should better run all unit test with `sudo`.

Note that if you run `ifconfig lo down` successfully, you should run `ifconfig lo up` again, otherwise, your network is closed and all the unit test will fail.

## Unit test in server mode

We run the unit test code as a server, so we need an external **client_for_server_test** as the client side.

Stop `sample_server` before running this test.

**NOTE** If you run `./rtUnitTest` with `sudo`, you should run all the commands in this section with `sudo` too.

### (1) run rtUnitTestServer

Switch to `pxCore/remote/tests/build` folder, then run `./rtUnitTestServer`

**NOTE** The `rtUnitTestServer` will be run about 60 seconds, so when you run it successfully, you should run `client_for_server_test` as soon as possible.

### (2) run client_for_server_test

Open the other terminal, switch to `pxCore/remote/tests/build` folder, then run `./client_for_server_test`.

Note that the `./client_for_server_test` will output some error messages, these errors are harmless and just ignored (These are due to the float check or double check).

**NOTE** The `client_for_server_test` will run about 30 seconds, so we can guarantee that the server is still running when the client is closing.

The `rtUnitTestServer` will end in about 60 seconds, then you can see the results like

```
[==========] Running 1 test from 1 test case.
[----------] Global test environment set-up.
[----------] 1 test from rtRpcRemoteServerTest
[ RUN      ] rtRpcRemoteServerTest.serverTest
rt: WARN rtRemoteStreamSelector.cpp:143 -- Thread-34875: error dispatching message. Transport endpoint is not connected
[       OK ] rtRpcRemoteServerTest.serverTest (61011 ms)
[----------] 1 test from rtRpcRemoteServerTest (61012 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test case ran. (61012 ms total)
[  PASSED  ] 1 test.
```

## Unit test in server mode (with unix scheme)

For covering the code about unix protocols, we have to run the "Unit test in server mode" with unix scheme again.

In `pxCore/remote/tests/build` folder, you can run
```
cp rtremote.conf.unix rtremote.conf
```

to switch the scheme to unix (from tcp).

Then repeat the steps in **Unit test in server mode** section again.

Please run this step twice to cover some unix socket cleanup code.

## generate code coverage report

in `pxCore/remote/tests/build` folder, run
```
./lcov.sh
```
Then you will get code coverage report in `result` folder.
you can browse index.html in `result` folder to get coverage report info.

I also provide the report result `<submission>/result` for your reference.

The result is

- Lines: 1839 / 2105 (87.4%)
- Functions: 252 / 269 (93.7%)

For individual files, only 3 files' coverage is less than 80%

- rtRemoteFactory.cpp - Lines: 68.8% (11 / 16), Functions: 50.0% (2 / 4)
- rtRemoteSocketUtils.cpp - Lines: (11 / 16) 77.3%, Functions: 90.0% (18 / 20)
- rtRemoteEnvironment.cpp - Lines: (86 / 114) 75.4%, Functions: 91.7% (11 / 12)

Here are the reasons:

- rtRemoteFactory.cpp
  - `rtRemoteFactory::rtRemoteFactory` and `rtRemoteFactory::~rtRemoteFactory` are private and not accessible outside. So it's impossible to cover them, and the ratio is low. Because there are only 16 lines and 4 functions.

- rtRemoteSocketUtils.cpp
  - The uncovered code is about IPv6 (`AF_INET6`), which is not easy to test. Anyway, the line coverage is closing to 80%.

- rtRemoteEnvironment.cpp
  - The working queue is difficult to mock, and this file was tested in the previous challenge, and the result is exactly same here.

## Remarks

### Tests description

#### Tests in rtRpcUnitTest.cpp

- method0AndReturn10Test
  - This test is used to test method method0AndReturn10 in remote. It test  
  **method.call.request** message. send value to remote and the server will return 10.

- method1AndReturnTest
  - This test is used to test method method1AndReturn in remote. It tests **method.call.request** message. send value to remote and the server will return cast int of 2*value.
  -
- twoIntNumberSumTest
  - This test is used to test method twoIntNumberSum in remote. It tests **method.call.request** message. send two values to remote and the server will
  return cast int of sum of this two values.

- twoFloatNumberSumTest
  - This test is used to test method twoFloatNumberSum in remote. It tests **method.call.request** message. send two values to remote and the server will
  return cast float of sum of this two values.

- method1IntAndNoReturnTest
  - This test is used to test method method1IntAndNoReturn in remote. It tests **method.call.request** message. send value to remote and get no response.

- method2FunctionAndNoReturnTest
  - This test is used to test method method2FunctionAndNoReturn in remote. It tests **method.call.request** message. send function to remote and get no response.

- setByIndexTest
  - test message set.byindex.request. objectRef.set(1, setVal) will return RT_OK.when client send set.byindex.request to server, it will return set.byname.response not set.byindex.response

- getByIndexTest
  - test message get.byindex.request. code bug: rtRemote client doesn't suport message get.byindex.request.

- setAndGetValueTest
  - test message set.byname.request and get.byname.request.

- errorValueTypeTest
  - test error value type. fffloat type is not set in the remote. when send fffloat type to remote, it will return RT_PROP_NOT_FOUND.

- notSetButGetValueTest
  - test not set string type value. string value is not set in the remote. When send get.byname.request to remote, it will return ""

- objectTypeTest
  - test object type value set and get.

- funcTypeTest
  - test function type value set and get

- floatTypeTest
  - test float type value set and get

- boolTypeTest
  - test bool type value set and get

- int8TypeTest
  - test int8 type value set and get.

- uint8TypeTest
  - test uint8 type value set and get.

- int32TypeTest
  - test int32 type value set and get.

- uint32TypeTest
  - test uint32 type value set and get.

- int64TypeTest
  - test int64 type value set and get.

- uint64TypeTest
  - test uint64 type value set and get.

- doubleTypeTest
  - test double type value set and get.

- stringTypeTest
  - test string type value set and get.

- voidPtrTypeTest
  - test voidPtr type value set and get.

#### Tests in rtConnectionTest.cpp

- failedConnectionTest
  - test failed connection

- successConnectionTest
  - test success connection

- searchObjectConnectionBrokenHandleTest
  - test  search  object when connection is broken

- sendSetConnectionBrokenHandleTest
  - test set value to remote when connection is broken

- sendGetConnectionBrokenHandleTest
  - test get value from remote when connection is broken

#### Tests in rtRemoteTest.cpp

- rtRemoteInitNoParamTest
  - test function rtRemoteInit()

- rtRemoteInitTest
  - test function rtRemoteInit(rtRemoteEnvironment* env)

- rtRemoteShutdownNoParamTest
  - test function rtRemoteShutdown()

- rtRemoteShutdownImmediateTest
  - test function rtRemoteShutdown, immediate shut down

- rtRemoteShutdownNotImmediateTest
  - test function rtRemoteShutdown, not immediate shut down

- rtRemoteRegisterObjectTest
  - test function rtRemoteRegisterObject

- rtRemoteRegisterObjectInvalidParamsTest
  - test invalid args to rtRemoteRegisterObject(rtRemoteEnvironment* env,
    char const* id, rtObjectRef const& obj)

- rtRemoteUnregisterObjectTest
  - test function rtRemoteUnregisterObject

- rtRemoteUnRegisterObjectInvalidParamsTest
  - test invalid args to rtRemoteRegisterObject

- rtRemoteLocateObjectInvalidParamsTest
  - test invalid args to rtRemoteLocateObject(rtRemoteEnvironment* env, char const* id, rtObjectRef& obj, int timeout, remoteDisconnectedCallback cb, void *cbdata)

- rtRemoteRunUntilTest
  - test function rtRemoteRunUntil

- rtEnvironmentFromFileTest
  - test function rtEnvironmentFromFile

- rtRemoteRegisterQueueReadyHandlerTest
  - test function rtRemoteRegisterQueueReadyHandler

#### Tests in rtRemoteEnvironmentTest.cpp

- waitForResponseTest
  - test function waitForResponse

#### Tests in rtRemoteEndPointTest.cpp

- stringTest
  - test `rtRemoteFileEndPoint::toString` and `rtRemoteEndPoint::fromString`

#### Tests in rtRemoteFactoryTest.cpp

- rtResolverTypeFromString
  - to cover other resolver types in `rtResolverTypeFromString`

#### Tests in rtRemoteMessageTest.cpp

- dumpDocumentTest
  - test `rtMessage_DumpDocument`

- getterSetterTest
  - test `rtMessage_setXXX` and `rtMessage_getXXX`

#### Tests in rtRpcRemoteServerTest.cpp

- serverTest
  - Run the server and wait for the client's test
