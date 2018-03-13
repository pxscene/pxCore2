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

#include "rtObject.h"
#include "rtString.h"
#include <string.h>
#include <unistd.h>
#include <pxScene2d.h>
#include <pxImage.h>

#include "test_includes.h" // Needs to be included last

using namespace std;

rtError callbackFn(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(numArgs);
  UNUSED_PARAM(args);
  UNUSED_PARAM(result);
  UNUSED_PARAM(context);
  return RT_OK;
}
rtFunctionCallback fnCallback(&callbackFn,NULL);

class rtEmitTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      mEmit = new rtEmit();
    }

    virtual void TearDown()
    {
    }

    void setListenerTest()
    {
      rtString event("eventone");
      EXPECT_TRUE (RT_OK == mEmit->setListener(event.cString(),&fnCallback));
    }

    void addListenerEmptyFnTest()
    {
      rtString event("eventone");
      EXPECT_TRUE (RT_ERROR == mEmit->addListener(event.cString(),NULL));
    }

    void addListenerDuplicateEventTest()
    {
      rtString event("eventone");
      EXPECT_TRUE (RT_OK == mEmit->addListener(event.cString(),&fnCallback));
    }

    void delListenerTest()
    {
      rtString event("eventone");
      EXPECT_TRUE (RT_OK == mEmit->delListener(event.cString(),&fnCallback));
    }

  private:
    rtEmit* mEmit;
};

TEST_F(rtEmitTest, rtEmitTests)
{
  setListenerTest();
  addListenerDuplicateEventTest();
  addListenerEmptyFnTest();
  delListenerTest();
}

class rtArrayObjectTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void emptyTest()
    {
      array.empty(); 
    }
  
    void getWithNameNullValFailedTest()
    {
      EXPECT_TRUE (RT_FAIL == array.Get("",NULL));
    }

    void getWithNameLenPropSuccessTest()
    {
      rtValue v;
      int64_t len;
      EXPECT_TRUE (RT_OK == array.Get("length",&v));
      v.getInt64(len);
      EXPECT_TRUE (0 == len);
    }

    void getWithNameWrongPropFailedTest()
    {
      rtValue v;
      EXPECT_TRUE (RT_PROP_NOT_FOUND == array.Get("empty",&v));
    }

    void getWithIdFailedTest()
    {
     EXPECT_TRUE (RT_FAIL == array.Get(1,NULL));
    }

    void getWithIdMoreIndexFailedTest()
    {
     rtValue v;
     v.setInt64(1);
     EXPECT_TRUE (RT_PROP_NOT_FOUND == array.Get(1,&v));
    }

    void setWithNameFailedTest()
    {
     rtValue v;
     EXPECT_TRUE (RT_PROP_NOT_FOUND == array.Set("entry",&v));
    }

    void setWithIdFailedTest()
    {
     EXPECT_TRUE (RT_ERROR_INVALID_ARG == array.Set(1,NULL));
    }

    void setWithIdPassedTest()
    {
     int index = 0;
     rtValue v;
     v.setInt64(1);
     EXPECT_TRUE (RT_OK == array.Set(index,&v));
    }

    private:
      rtArrayObject array;
};

TEST_F(rtArrayObjectTest, rtArrayObjectTests)
{
  emptyTest();
  getWithNameNullValFailedTest();
  getWithNameLenPropSuccessTest();
  getWithNameWrongPropFailedTest();
  getWithIdFailedTest();
  getWithIdMoreIndexFailedTest();
  setWithNameFailedTest();
  setWithIdFailedTest();
  setWithIdPassedTest();
}

class rtObjectTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void allKeysTest()
    {
      pxImage obj(NULL);
      rtObjectRef op;
      obj.allKeys(op);
      rtArrayObject* array = (rtArrayObject*) op.getPtr();
      EXPECT_TRUE(0 < array->length());
    }

    void getValByIndexTest()
    {
      rtObject obj;
      EXPECT_TRUE (RT_PROP_NOT_FOUND == obj.Get(1,NULL));
    }
 
    void setValWithIdFailedTest()
    {
      rtObject obj;
      EXPECT_TRUE (RT_PROP_NOT_FOUND == obj.Set(1,NULL));
    }

    void sendTests()
    {
      rtObject obj;
      rtError e = obj.send("exampleMessage");
      e = obj.send("exampleMessage", rtValue());
      e = obj.send("exampleMessage", rtValue(), rtValue());
      e = obj.send("exampleMessage", rtValue(), rtValue(), rtValue());
      e = obj.send("exampleMessage", rtValue(), rtValue(), rtValue(), rtValue());
      e = obj.send("exampleMessage", rtValue(), rtValue(), rtValue(), rtValue(), rtValue());
      EXPECT_TRUE (RT_OK != e);
    }

    void sendReturnsTests()
    {
      rtObject obj;
      rtString result;
      rtError e = obj.sendReturns("exampleMessage", result);
      e = obj.sendReturns("exampleMessage", rtValue(), result);
      e = obj.sendReturns("exampleMessage", rtValue(), rtValue(), result);
      e = obj.sendReturns("exampleMessage", rtValue(), rtValue(), rtValue(), result);
      e = obj.sendReturns("exampleMessage", rtValue(), rtValue(), rtValue(), rtValue(), result);
      e = obj.sendReturns("exampleMessage", rtValue(), rtValue(), rtValue(), rtValue(), rtValue(), result);
      e = obj.sendReturns("exampleMessage", rtValue(), rtValue(), rtValue(), rtValue(), rtValue(), rtValue(), result);
      e = obj.sendReturns("exampleMessage", rtValue(), rtValue(), rtValue(), rtValue(), rtValue(), rtValue(), rtValue(), result);
      EXPECT_TRUE (RT_OK != e);
    }
};

TEST_F(rtObjectTest, rtObjectTests)
{
  allKeysTest();
  getValByIndexTest();
  setValWithIdFailedTest();
  sendTests();
  sendReturnsTests();
}

class rtMapObjectTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void setValByIndexTest()
    {
      rtMapObject obj;
      EXPECT_TRUE (RT_PROP_NOT_FOUND == obj.Set(1,NULL));
    }

    void getValByIndexTest()
    {
      rtMapObject obj;
      EXPECT_TRUE (RT_PROP_NOT_FOUND == obj.Get(1,NULL));
    }

    void setValByIndexWithEmptyValTest()
    {
      rtMapObject obj;
      EXPECT_TRUE (RT_FAIL == obj.Set("entry",NULL));
    }
};

TEST_F(rtMapObjectTest, rtMapObjectTests)
{
  setValByIndexTest();
  getValByIndexTest();
  setValByIndexWithEmptyValTest();
}

