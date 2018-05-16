/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the tests in this file are used to test function
 * of rtRemoteFactory.cpp
 *
 * @author      TCSCODER
 * @version     1.0
 */
#include "rtRemoteConfig.h"
#include "rtRemoteFactory.h"
#include "rtRemoteEnvironment.h"
#include "rtTestIncludes.h"

#include <string>

/**
 * rtRemoteFactoryTest class
 */
class rtRemoteFactoryTest : public ::testing::Test
{
protected:
 virtual void SetUp() {
 }
 virtual void TearDown() {
 }
};

/**
 * To cover other resolver types in rtResolverTypeFromString
 */
TEST_F(rtRemoteFactoryTest, rtResolverTypeFromString){
  // Construct a mock rtRemoteEnvironment for test "file" resolver type
  rtRemoteConfig configFile;
  configFile.set_resolver_type("file");
  rtRemoteEnvironment* envFile(new rtRemoteEnvironment(&configFile));
  EXPECT_EQ(nullptr, rtRemoteFactory::rtRemoteCreateResolver(envFile));

  // Construct a mock rtRemoteEnvironment for test "unicast" resolver type
  rtRemoteConfig configUnicast;
  configUnicast.set_resolver_type("unicast");
  rtRemoteEnvironment* envUnicast(new rtRemoteEnvironment(&configUnicast));
  EXPECT_EQ(nullptr, rtRemoteFactory::rtRemoteCreateResolver(envUnicast));
}
