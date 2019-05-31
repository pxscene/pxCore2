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

namespace
{
  const char* envTestBaseFilePathVal = "TEST_BASE_FILE_PATH_VAL";
}

class baseFilePathTest : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void test_relativePath()
  {
    pxScriptView* view = new pxScriptView("supportfiles/baseFilePath.js", "");
    process(view, .5f);
    view->onCloseRequest();
    delete view;

    rtString env = rtGetEnvAsString(envTestBaseFilePathVal);
    EXPECT_EQ (std::string(env.cString()), "supportfiles");
  }

  void test_relativePathDot()
  {
    pxScriptView* view = new pxScriptView("./supportfiles/baseFilePath.js", "");
    process(view, .5f);
    view->onCloseRequest();
    delete view;

    rtString env = rtGetEnvAsString(envTestBaseFilePathVal);
    EXPECT_EQ (std::string(env.cString()), "./supportfiles");
  }

  void test_relativePathDoubleDot()
  {
    pxScriptView* view = new pxScriptView("../../tests/pxScene2d/supportfiles/baseFilePath.js", "");
    process(view, .5f);
    view->onCloseRequest();
    delete view;

    rtString env = rtGetEnvAsString(envTestBaseFilePathVal);
    EXPECT_EQ (std::string(env.cString()), "../../tests/pxScene2d/supportfiles");
  }

  void test_absolutePath()
  {
    rtString d;
    EXPECT_EQ ((int)RT_OK, (int)rtGetCurrentDirectory(d));
    d += "/supportfiles";
    rtString file(d);
    file += "/baseFilePath.js";

    pxScriptView* view = new pxScriptView(file, "");
    process(view, .5f);
    view->onCloseRequest();
    delete view;

    rtString env = rtGetEnvAsString(envTestBaseFilePathVal);
    EXPECT_EQ (std::string(env.cString()), d.cString());
  }

  void test_fileURL()
  {
    rtString d;
    EXPECT_EQ ((int)RT_OK, (int)rtGetCurrentDirectory(d));
    d = "file://" + d + "/supportfiles";
    rtString file(d);
    file += "/baseFilePath.js";

    pxScriptView* view = new pxScriptView(file, "");
    process(view, .5f);
    view->onCloseRequest();
    delete view;

    rtString env = rtGetEnvAsString(envTestBaseFilePathVal);
    EXPECT_EQ (std::string(env.cString()), d.cString());
  }

  void test_urlInQuery()
  {
    pxScriptView* view = new pxScriptView("supportfiles/baseFilePath.js?tests=file:///home/tests.json", "");
    process(view, .5f);
    view->onCloseRequest();
    delete view;

    rtString env = rtGetEnvAsString(envTestBaseFilePathVal);
    EXPECT_EQ (std::string(env.cString()), "supportfiles");
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

TEST_F(baseFilePathTest, baseFilePathTests)
{
  test_relativePath();
  test_relativePathDot();
  test_relativePathDoubleDot();
  test_absolutePath();
  test_fileURL();
  test_urlInQuery();
}
