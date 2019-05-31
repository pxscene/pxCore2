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

#ifndef ENABLE_RT_NODE
#define ENABLE_RT_NODE
#endif

#include <sstream>

#define private public
#define protected public

#include "pxScene2d.h"
#include <string.h>
#include "pxIView.h"
#include "pxTimer.h"
#include "rtObject.h"
#include <rtRef.h>

#include "test_includes.h" // Needs to be included last

using namespace std;

extern rtScript script;

const char* paramName = "testparam";
const char* paramNameZero = "0";

class pxSceneObjectWrapperTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void setObjectNameTest()
    {
      startJsFile("supportfiles/objectname.js");
      process();
      populateObjects();
      pxSceneContainer* sceneContainer = mSceneContainer[0];
      rtValue v;
      rtError err = sceneContainer->api(v); 
      rtObjectRef ref = v.toObject();
      rtValue val(1);
      err = ref.Set(paramName, &val);
      EXPECT_TRUE (RT_OK == err);
      script.collectGarbage();
    }

    void getObjectEmptyNameTest()
    {
      startJsFile("supportfiles/objectname.js");
      process();
      populateObjects();
      pxSceneContainer* sceneContainer = mSceneContainer[0];
      rtValue v;
      rtError err = sceneContainer->api(v); 
      rtObjectRef ref = v.toObject();
      rtValue val(1);
      err = ref.Get((const char*)NULL, &val);
      EXPECT_TRUE (RT_ERROR_INVALID_ARG == err);
      script.collectGarbage();
    }

    void getObjectEmptyValueTest()
    {
      startJsFile("supportfiles/objectname.js");
      process();
      populateObjects();
      pxSceneContainer* sceneContainer = mSceneContainer[0];
      rtValue v;
      rtError err = sceneContainer->api(v); 
      rtObjectRef ref = v.toObject();
      rtValue val(1);
      err = ref.Get(paramName, NULL);
      EXPECT_TRUE (RT_ERROR_INVALID_ARG == err);
      script.collectGarbage();
    }

    void setObjectNameAsArrayTest()
    {
      startJsFile("supportfiles/objectindex.js");
      process();
      populateObjects();
      pxSceneContainer* sceneContainer = mSceneContainer[0];
      rtValue v;
      rtError err = sceneContainer->api(v);
      rtObjectRef ref = v.toObject();
      rtValue val(1);
      err = ref.Set(paramNameZero, &val);
      EXPECT_TRUE (RT_OK == err);
      script.collectGarbage();
    }


    void setObjectNameInvalidParamsTest()
    {
      startJsFile("supportfiles/objectname.js");
      process();
      populateObjects();
      pxSceneContainer* sceneContainer = mSceneContainer[0];
      rtValue v;
      rtError err = sceneContainer->api(v);
      rtObjectRef ref = v.toObject();
      rtValue val(1);
      err = ref.Set((char *)NULL, &val);
      EXPECT_TRUE (RT_ERROR_INVALID_ARG == err);
      err = ref.Set(paramName, NULL);
      EXPECT_TRUE (RT_ERROR_INVALID_ARG == err);
      script.collectGarbage();
    }

    void setObjectIndexTest()
    {
      startJsFile("supportfiles/objectindex.js");
      process();
      populateObjects();
      pxSceneContainer* sceneContainer = mSceneContainer[0];
      rtValue v;
      rtError err = sceneContainer->api(v); 
      rtObjectRef ref = v.toObject();
      rtValue val(1);
      err = ref.Set(3, &val);
      EXPECT_TRUE (RT_OK == err);
      script.collectGarbage();
    }

    void getObjectEmptyValueIndexTest()
    {
      startJsFile("supportfiles/objectindex.js");
      process();
      populateObjects();
      pxSceneContainer* sceneContainer = mSceneContainer[0];
      rtValue v;
      rtError err = sceneContainer->api(v);
      rtObjectRef ref = v.toObject();
      rtValue val(1);
      err = ref.Get(3, NULL);
      EXPECT_TRUE (RT_ERROR_INVALID_ARG == err);
      script.collectGarbage();
    }

    void getObjectIndexNotFoundTest()
    {
      startJsFile("supportfiles/objectindex.js");
      process();
      populateObjects();
      pxSceneContainer* sceneContainer = mSceneContainer[0];
      rtValue v;
      rtError err = sceneContainer->api(v);
      rtObjectRef ref = v.toObject();
      rtValue val(1);
      err = ref.Get(100, &val);
      EXPECT_TRUE (RT_PROPERTY_NOT_FOUND == err);
      script.collectGarbage();
    }

    void setObjectIndexInvalidParamsTest()
    {
      startJsFile("supportfiles/objectindex.js");
      process();
      populateObjects();
      pxSceneContainer* sceneContainer = mSceneContainer[0];
      rtValue v;
      rtError err = sceneContainer->api(v);
      rtObjectRef ref = v.toObject();
      rtValue val(1);
      err = ref.Set(3, NULL);
      EXPECT_TRUE (RT_ERROR_INVALID_ARG == err);
      script.collectGarbage();
    }

private:

    void startJsFile(const char *jsfile)
    {
      mUrl = jsfile;
      mView = new pxScriptView(mUrl,"");
    }

    void process()
    {
      double  secs = pxSeconds();
      while ((pxSeconds() - secs) < 1.0)
      {
        if (NULL != mView)
        {
          mView->onUpdate(pxSeconds());
          script.pump();
        }
      }
    }

    void populateObjects()
    {
      rtObjectRef scene = mView->mScene;
      pxScene2d* sceneptr = (pxScene2d*)scene.getPtr();
      mRoot = sceneptr->getRoot();
      int objcount = 0;
      for(vector<rtRefT<pxObject> >::iterator it = mRoot->mChildren.begin(); it != mRoot->mChildren.end(); ++it)
      {
        if (strcmp((*it)->getMap()->className,"pxSceneContainer") == 0)
        {
          mSceneContainer[objcount] = (pxSceneContainer*) (*it).getPtr();
        }
      }
    }

    pxObject* mRoot;
    pxSceneContainer* mSceneContainer[100];
    pxScriptView* mView;
    rtString mUrl;
};

TEST_F(pxSceneObjectWrapperTest, ObjectWrapperTest)
{
  setObjectNameTest();
  setObjectNameAsArrayTest();
  setObjectNameInvalidParamsTest();
  setObjectIndexTest();
  setObjectIndexInvalidParamsTest();
  getObjectEmptyNameTest();
  getObjectEmptyValueTest();
  getObjectEmptyValueIndexTest();
  getObjectIndexNotFoundTest();
}
