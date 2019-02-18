/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <sstream>

#define private public
#define protected public

#include "pxScene2d.h"
#include "pxTimer.h"
#include "rtPathUtils.h"

#include "test_includes.h" // Needs to be included last

extern rtScript script;

class httpEndAfterCloseTest : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void runTestFor2s()
  {
    rtString env;
    env = rtGetEnvAsString("TEST_HTTP_5S_VAL");
    EXPECT_EQ (std::string(env.cString()), "");

    script.collectGarbage();
    pxScriptView* view = new pxScriptView("supportfiles/http5s.js?val=a", "");
    process(view, 2.f);
    view->onCloseRequest();
    delete view;
    script.collectGarbage();

    env = rtGetEnvAsString("TEST_HTTP_5S_VAL");
    EXPECT_EQ (std::string(env.cString()), "");
  }

  void runTestFor8s()
  {
    rtString env;
    env = rtGetEnvAsString("TEST_HTTP_5S_VAL");
    EXPECT_EQ (std::string(env.cString()), "");

    script.collectGarbage();
    pxScriptView* view = new pxScriptView("supportfiles/http5s.js?val=b", "");
    process(view, 8.f);
    view->onCloseRequest();
    delete view;
    script.collectGarbage();

    env = rtGetEnvAsString("TEST_HTTP_5S_VAL");
    EXPECT_EQ (std::string(env.cString()), "b");
  }

private:
  void process(pxScriptView* view, float timeout)
  {
    double secs = pxSeconds();
    while ((pxSeconds() - secs) < timeout)
    {
      view->onUpdate(pxSeconds());
      script.pump();
    }
  }
};

TEST_F(httpEndAfterCloseTest, httpEndAfterCloseTests)
{
  runTestFor2s();

  pxSleepMS(3000); // response ETA
  pxSleepMS(2000); // wait a bit

  runTestFor8s();
}
