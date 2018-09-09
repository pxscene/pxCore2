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

#include <rtError.h>
#include "rtString.h"
#include <string.h>
#include <map>

#include "test_includes.h" // Needs to be included last


using namespace std;

#define RT_ERROR_CLASS(ERR) (((ERR) >> 16) & 0x0000ffff)
map <int, const char*> rtBuiltinErrors;

class rtErrorTest : public ::testing::TestWithParam<rtError>
{
  public:
    virtual void SetUp()
    {
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_OK, "RT_OK"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_FAIL, "RT_FAIL"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_NOT_ENOUGH_ARGS, "RT_ERROR_NOT_ENOUGH_ARGS"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_INVALID_ARG, "RT_ERROR_INVALID_ARG"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_PROP_NOT_FOUND, "RT_PROP_NOT_FOUND"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_OBJECT_NOT_INITIALIZED, "RT_OBJECT_NOT_INITIALIZED"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_PROPERTY_NOT_FOUND, "RT_PROPERTY_NOT_FOUND"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_OBJECT_NO_LONGER_AVAILABLE, "RT_OBJECT_NO_LONGER_AVAILABLE"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_RESOURCE_NOT_FOUND, "RT_RESOURCE_NOT_FOUND"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_NO_CONNECTION, "RT_NO_CONNECTION"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_NOT_IMPLEMENTED, "RT_ERROR_NOT_IMPLEMENTED"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_TYPE_MISMATCH, "RT_ERROR_TYPE_MISMATCH"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_TIMEOUT, "RT_ERROR_TIMEOUT"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_DUPLICATE_ENTRY, "RT_ERROR_DUPLICATE_ENTRY"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_OBJECT_NOT_FOUND, "RT_ERROR_OBJECT_NOT_FOUND"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_PROTOCOL_ERROR, "RT_ERROR_PROTOCOL_ERROR"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_INVALID_OPERATION, "RT_ERROR_INVALID_OPERATION"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_IN_PROGRESS, "RT_ERROR_IN_PROGRESS"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_QUEUE_EMPTY, "RT_ERROR_QUEUE_EMPTY"));
      rtBuiltinErrors.insert (std::pair <const int, const char*> (RT_ERROR_STREAM_CLOSED, "RT_ERROR_STREAM_CLOSED"));
    }

    virtual void TearDown()
    {
      rtBuiltinErrors.clear();
    }
};

TEST_P(rtErrorTest, rtStrErrorTest) {
  rtError e = GetParam();
  string error = rtStrError(e);
  if ( RT_ERROR_CLASS(e) == RT_ERROR_CLASS_BUILTIN)
  {
    if ( rtBuiltinErrors.find(e) != rtBuiltinErrors.end() )
      EXPECT_TRUE ( strcmp(error.c_str(), rtBuiltinErrors[e]) == 0 );
    else
      EXPECT_TRUE (strcmp(error.c_str(), "UNKNOWN") == 0);
  }
  else
  {
    #ifdef __linux__
      EXPECT_TRUE (strlen(error.c_str()) > 0);
    #else
      // currently strerror is not returning data for platform other than linux, so considering it as zero length string
      EXPECT_TRUE (strlen(error.c_str()) == 0);
    #endif
  }
}


INSTANTIATE_TEST_CASE_P(rtStrErrorApi, rtErrorTest, ::testing::Values(RT_OK, RT_FAIL, RT_ERROR_NOT_ENOUGH_ARGS, RT_ERROR_INVALID_ARG, RT_PROP_NOT_FOUND, RT_OBJECT_NOT_INITIALIZED, RT_PROPERTY_NOT_FOUND, RT_OBJECT_NO_LONGER_AVAILABLE, RT_RESOURCE_NOT_FOUND, RT_NO_CONNECTION, RT_ERROR_NOT_IMPLEMENTED, RT_ERROR_TYPE_MISMATCH, RT_ERROR_TIMEOUT, RT_ERROR_DUPLICATE_ENTRY, RT_ERROR_OBJECT_NOT_FOUND, RT_ERROR_PROTOCOL_ERROR, RT_ERROR_INVALID_OPERATION, RT_ERROR_IN_PROGRESS, RT_ERROR_QUEUE_EMPTY, RT_ERROR_STREAM_CLOSED, 9999, 2147483648));
