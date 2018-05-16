/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the tests in this file are used to test function
 * of rtRemoteEnvironment.cpp
 *
 * @author      TCSCODER
 * @version     1.0
 */

#include "rtRemoteEnvironment.h"
#include "rtRemoteConfig.h"
#include "rtRemoteMessage.h"
#include "rtTestIncludes.h"

/**
 * rtRemoteEnvironmentTest class
 */
class rtRemoteEnvironmentTest : public ::testing::Test
{
protected:
 virtual void SetUp() {
 }
 virtual void TearDown() {
 }
};
/**
 * test waitForResponse
 */
TEST(rtRemoteEnvironmentTest, waitForResponseTest){
  std::chrono::milliseconds timeout = std::chrono::milliseconds(3000u);
  rtRemoteCorrelationKey m_id = rtMessage_GetNextCorrelationKey();
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  /**
   * wait 3000ms for response, because no send data to remote,
   * so no response from remote, it will return RT_ERROR_TIMEOUT
   */
  EXPECT_EQ(RT_ERROR_TIMEOUT, env->waitForResponse(timeout, m_id));
  EXPECT_EQ(RT_OK, rtRemoteShutdown());
}
