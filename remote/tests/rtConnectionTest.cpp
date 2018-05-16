/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the test cases in this file are used to
 * test failed connection,success connection
 * and test robustness,ensuring that if a connection is broken
 * that the app behaves in a predictable way
 *
 * @author      TCSCODER
 * @version     1.0
 */
#include "rtTestIncludes.h"

/**
 * rtConnectionTest class
 */
class rtConnectionTest : public ::testing::Test {
protected:
 /**
  * Init environment
  */
 virtual void SetUp() {
   EXPECT_EQ(RT_OK, rtRemoteInit());
 }
 /**
  * clear environment
  */
 virtual void TearDown() {
   EXPECT_EQ(RT_OK, rtRemoteShutdown());
 }
 rtObjectRef objectRef;
 rtString registeredObjectName = "host_object";
};

/**
 * test failed connection
 * use unregistered object id to search object in remote,
 * the connection will fail, it will return RT_FAIL
 */

TEST_F(rtConnectionTest, failedConnectionTest){
  rtString unregisteredObjectName = "test"; // unregisted object in the remote
  rtObjectRef obj;
  EXPECT_EQ(RT_FAIL, rtRemoteLocateObject(unregisteredObjectName, obj));
}

/**
 * test success connection
 * use registered object id to search object in remote,
 * the connecton will succeeded, it will return RT_OK
 */
TEST_F(rtConnectionTest, successConnectionTest){
  rtObjectRef obj;
  EXPECT_EQ(RT_OK, rtRemoteLocateObject(registeredObjectName, obj));
}

/**
 * test  search  object when connection is broken
 */
TEST_F(rtConnectionTest, searchObjectConnectionBrokenHandleTest){
  int timeOut = 3000;
  system("ifconfig lo down");
  // if connection is broken, rtRemoteLocateObject should return RT_ERROR after 3000ms
  EXPECT_EQ(RT_ERROR, rtRemoteLocateObject(registeredObjectName, objectRef, timeOut));
  system("ifconfig lo up");
}

/**
 * test set value to remote when connection is broken
 */
TEST_F(rtConnectionTest, sendSetConnectionBrokenHandleTest){
  int value = 100;
  EXPECT_EQ(RT_OK, rtRemoteLocateObject(registeredObjectName, objectRef));
  system("ifconfig lo down");
  // if connction is broken, it will return timeout error
  EXPECT_EQ(RT_ERROR_TIMEOUT, objectRef.set("count", value));
  system("ifconfig lo up");
}

/**
 * test get value from remote when connection is broken
 */
TEST_F(rtConnectionTest, sendGetConnectionBrokenHandleTest){
  int value = 100;
  rtValue newVal;
  EXPECT_EQ(RT_OK, rtRemoteLocateObject(registeredObjectName, objectRef));
  EXPECT_EQ(RT_OK, objectRef.set("count", value));
  system("ifconfig lo down");
  // if connction is broken, it will return timeout error
  EXPECT_EQ(RT_ERROR_TIMEOUT, objectRef.get("count", newVal));
  system("ifconfig lo up");
  // expected value should be not equal actual value
  EXPECT_NE(value, newVal.toInt32());
}
