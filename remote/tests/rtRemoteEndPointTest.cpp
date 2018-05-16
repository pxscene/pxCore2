/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the tests in this file are used to test function
 * of rtRemoteEndPoint.cpp
 *
 * @author      TCSCODER
 * @version     1.0
 */
#include "rtRemoteConfig.h"
#include "rtRemoteEndPoint.h"
#include "rtTestIncludes.h"

#include <string>

/**
 * rtRemoteEndPointTest class
 */
class rtRemoteEndPointTest : public ::testing::Test
{
protected:
 virtual void SetUp() {
 }
 virtual void TearDown() {
 }
};

#define TEST_SCHEME "unix"
#define TEST_PATH "/home/root/test"

/**
 * test rtRemoteFileEndPoint::toString and rtRemoteEndPoint::fromString
 */
TEST_F(rtRemoteEndPointTest, stringTest){
  const char* testUrl = TEST_SCHEME "://" TEST_PATH;
  // test rtRemoteEndPoint::fromString
  rtRemoteFileEndPoint* result = dynamic_cast<rtRemoteFileEndPoint*>(rtRemoteEndPoint::fromString(testUrl));
  EXPECT_STREQ(TEST_SCHEME, result->scheme().c_str());
  EXPECT_STREQ(TEST_PATH, result->path().c_str());

  // test rtRemoteFileEndPoint::rtRemoteToString
  const std::string& fileEndPointStr = result->toString();
  EXPECT_STREQ(testUrl, fileEndPointStr.c_str());

  // This point is dynamically created, delete it to free the memeory.
  delete result;
}
