/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the tests in this file are used to test function
 * of rtRemoteMessage.cpp
 *
 * @author      TCSCODER
 * @version     1.0
 */
#include "rtRemoteConfig.h"
#include "rtRemoteMessage.h"
#include "rtTestIncludes.h"

#include <string>

/**
 * rtRemoteMessageTest class
 */
class rtRemoteMessageTest : public ::testing::Test
{
protected:
 virtual void SetUp() {
 }
 virtual void TearDown() {
 }
};

/**
 * test rtMessage_DumpDocument
 */
TEST_F(rtRemoteMessageTest, dumpDocumentTest){
  // Create a mock rtRemoteMessage object for testing
  rtRemoteMessagePtr req(new rtRemoteMessage());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeOpenSessionRequest, req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, "test.correlation.key", req->GetAllocator());
  req->AddMember(kFieldNameObjectId, "test.objectId", req->GetAllocator());

  // Dump the document to stdout
  EXPECT_EQ(RT_OK, rtMessage_DumpDocument(*req, nullptr));
}

/**
 * test rtMessage_setXXX and rtMessage_getXXX
 * For example:
 *  - rtMessage_GetPropertyIndex
 *  - rtMessage_GetStatusCode
 *  - rtMessage_GetStatusMessage
 *  - rtMessage_SetStatus
 */
TEST_F(rtRemoteMessageTest, getterSetterTest){
  // Create a mock rtRemoteMessage object for testing
  rtRemoteMessagePtr req(new rtRemoteMessage());
  req->SetObject();
  req->AddMember(kFieldNameMessageType, kMessageTypeOpenSessionRequest, req->GetAllocator());
  req->AddMember(kFieldNameCorrelationKey, "test.correlation.key", req->GetAllocator());
  const char* testObjectId = "test.objectId";
  req->AddMember(kFieldNameObjectId, std::string(testObjectId), req->GetAllocator());
  uint32_t testPropertyIndex = 123456;
  req->AddMember(kFieldNamePropertyIndex, testPropertyIndex, req->GetAllocator());

  // Get property name
  EXPECT_EQ(NULL, rtMessage_GetPropertyName(*req));
  // Add a new property name and test
  const char* testPropertyName = "test.property.name";
  req->AddMember(kFieldNamePropertyName, std::string(testPropertyName), req->GetAllocator());
  EXPECT_STREQ(testPropertyName, rtMessage_GetPropertyName(*req));

  // Get property index
  EXPECT_EQ(testPropertyIndex, rtMessage_GetPropertyIndex(*req));

  // Get object id
  EXPECT_STREQ(testObjectId, rtMessage_GetObjectId(*req));

  // Get status code (for testing dumpDoc)
  EXPECT_EQ(RT_FAIL, rtMessage_GetStatusCode(*req));
  
  // Get status message
  EXPECT_EQ(NULL, rtMessage_GetStatusMessage(*req));

  // Set status code
  EXPECT_EQ(RT_OK, rtMessage_SetStatus(*req, RT_FAIL));
  // Test get status code again
  EXPECT_EQ(RT_FAIL, rtMessage_GetStatusCode(*req));
  // Set status code with format string
  const char* testStatusMessage = "test.status.message";
  EXPECT_EQ(RT_OK, rtMessage_SetStatus(*req, RT_OK, "%s", testStatusMessage));
  // Test status message again.
  EXPECT_STREQ(testStatusMessage, rtMessage_GetStatusMessage(*req));
}