/*

Copyright 2018 Damian Wrobel <dwrobel@ertelnet.rybnik.pl>

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

#include "rtPathUtils.h"

#include "test_includes.h" // Needs to be included last

using namespace std;

class rtPathUtilsTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void concatenatePath()
    {
      std::string s1 = rtConcatenatePath("foo", "bar");
      rtString s2 = "foo";

      rtError rv = rtEnsureTrailingPathSeparator(s2);
      EXPECT_EQ(rv, RT_OK);
      s2.append("bar");

      EXPECT_EQ(0, s1.compare(s2));
    }

    void getPutEnv()
    {
      rtString env = rtGetEnvAsString(env_name);
      EXPECT_EQ(0, strcmp(env, ""));

      const char *env_sample_value = "SampleValue";
      rtError rv = rtPathUtilPutEnv(env_name, env_sample_value);
      EXPECT_EQ(rv, RT_OK);

      env = rtGetEnvAsString(env_name);
      EXPECT_EQ(0, strcmp(env, env_sample_value));

      rv = rtPathUtilPutEnv(env_name, "");
      EXPECT_EQ(rv, RT_OK);
    }

    void defaultValue()
    {
      rtModuleDirs::destroy(); // destroys any existing environment
      const int rv = rtPathUtilPutEnv(env_name, NULL);

      EXPECT_EQ(0, rv);

      rtString cwd;
      rtError rv1 = rtGetCurrentDirectory(cwd);

      EXPECT_EQ(0, rv1);
      EXPECT_EQ(false, cwd.isEmpty());

      int num_entries = 0;
      rtModuleDirs *p = rtModuleDirs::instance(env_name); // creates new environment

      for (rtModuleDirs::iter it = p->iterator(); it.first != it.second; it.first++)
      {
        const std::string s = *it.first;

        EXPECT_EQ(0, s.compare(cwd.cString()));
        num_entries++;
      }

      EXPECT_EQ(1, num_entries);

      rtModuleDirs::destroy(); // destroys environment

      rtModuleDirs::instance(); // re-creates environment
    }

    void basicTest()
    {
      const char *dir_entries[] = {
#       ifdef WIN32
        "e:\\foo",
#       endif
        "/bar",
        "baz/",
        "/qux/"
      };

      std::string env_str;

      for (size_t i = 0; i < sizeof (dir_entries) / sizeof(dir_entries[0]); i++)
      {
        if (i > 0)
          env_str += rtModuleDirSeparator();

        env_str += dir_entries[i];
      }

      rtModuleDirs::destroy(); // destroys any existing environment
      const int rv = rtPathUtilPutEnv(env_name, env_str.c_str());

      EXPECT_EQ(0, rv);

      size_t num_entries = 0;
      rtModuleDirs *p = rtModuleDirs::instance(env_name); // creates new environment

      for (rtModuleDirs::iter it = p->iterator(); it.first != it.second; it.first++)
      {
        const std::string s = *it.first;
        EXPECT_EQ(0, s.compare(dir_entries[num_entries]));
        num_entries++;
      }

      EXPECT_EQ(sizeof (dir_entries) / sizeof(dir_entries[0]), num_entries);

      rtModuleDirs::destroy(); // destroys environment

      rtModuleDirs::instance(); // re-creates environment
    }

    private:
      const char *env_name = "_pxscene_nie_calkiem_przypadkowy_env";
};

TEST_F(rtPathUtilsTest, rtPathUtilsTest)
{
  concatenatePath();
  getPutEnv();
  defaultValue();
  basicTest();
}

