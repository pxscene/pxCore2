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

class js2rtTest : public testing::Test
{
public:
  virtual void SetUp()
  {
    view = new pxScriptView("supportfiles/js2rt.js", "");
    process(1.f);
    scene = (pxScene2d*)view->mScene.getPtr();
  }

  virtual void TearDown()
  {
    view->onCloseRequest();
    delete view;
  }

  void test_boolean()
  {
    bool r = false;
    bool b = false;
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<bool>("b", b));
    EXPECT_TRUE (b);
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<bool>("b_", b, r));
    EXPECT_TRUE (r);
  }

  void test_string()
  {
    bool r = false;
    rtString s;
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<rtString>("s", s));
    EXPECT_EQ   (std::string("str"), s.cString());
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<bool>("s_", s, r));
    EXPECT_TRUE (r);
  }

  void test_integer()
  {
    bool r = false;
    int i = 0;
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<int>("i", i));
    EXPECT_EQ   ((int)123, i);
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<bool>("i_", i, r));
    EXPECT_TRUE (r);
  }

  void test_double()
  {
    bool r = false;
    double d = 0.0;
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<double>("d", d));
    EXPECT_EQ   ((double)1.23, d);
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<bool>("d_", d, r));
    EXPECT_TRUE (r);
  }

  void test_function()
  {
    bool r = false;
    rtFunctionRef fn;
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<rtFunctionRef>("fn", fn));
    EXPECT_TRUE (fn != NULL);
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<bool>("fn_", fn, r));
    //EXPECT_TRUE (r); // TODO: fails. why??
  }

  void test_object()
  {
    bool r = false;
    rtObjectRef o;
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<rtObjectRef>("o", o));
    EXPECT_TRUE (o != NULL);
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<bool>("o_", o, r));
    EXPECT_TRUE (r);
  }

  void test_promise()
  {
    bool r = false;
    rtObjectRef p;
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<rtObjectRef>("p", p));
    EXPECT_TRUE (p != NULL);
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<bool>("p_", p, r));
    EXPECT_TRUE (r);
  }

  void test_buffer()
  {
    bool r = false;
    rtObjectRef buf;
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<rtObjectRef>("buf", buf));
    EXPECT_TRUE (buf != NULL);
    EXPECT_EQ   ((int)RT_OK, (int)scene->mEmit.sendReturns<bool>("buf_", buf, r));
    EXPECT_TRUE (r);
  }

private:
  void process(float timeout)
  {
    double secs = pxSeconds();
    while ((pxSeconds() - secs) < timeout)
    {
      view->onUpdate(pxSeconds());
      script.pump();
    }
  }

  pxScriptView* view;
  pxScene2d* scene;
};

TEST_F(js2rtTest, js2rtTests)
{
  test_boolean();
  test_string();
  test_integer();
  test_double();
  test_function();
  test_object();
  test_promise();
  test_buffer();
}
