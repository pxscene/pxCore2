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

#include "rtRemote.h"
#include "rtObject.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

// design a remote object
// 1.) MUST derive from rtObject
// 2.) delclare a properties
// 3.) provide public impl for properties
// 4.) delcare stubs for properties and class
class ContinuousVideoRecorder : public rtObject
{
  // standard declarations
  rtDeclareObject(ContinuousVideoRecorder, rtObject);

public:

  ContinuousVideoRecorder()
    : _n(1234)
  {
  }

  // define a few properties
  // porperty name is "name"
  // getter is getName
  // setter is setName
  // type is rtString
  rtProperty(name, getName, setName, rtString);
  rtError getName(rtString& s) const
  {
    s = m_name;
    return RT_OK;
  }
  rtError setName(rtString const& s)
  {
    m_name = s;
    return RT_OK;
  }

  // property as rtObject
  rtProperty(bigprop, getBigProp, setBigProp, rtObjectRef);
  rtError getBigProp(rtObjectRef& obj) const
  {
    obj = m_big;
    return RT_OK;
  }
  rtError setBigProp(rtObjectRef const& obj)
  {
    m_big = obj;
    return RT_OK;
  }

  rtProperty(prop, getProp, setProp, int);
  rtError getProp(int& n) const { n = _n; return RT_OK; }
  rtError setProp(int  n) { _n = n; return RT_OK; }

  // properties can be functions too!
  rtProperty(onUploadComplete, getOnUploadComplete, setOnUploadComplete, rtFunctionRef);
  rtError getOnUploadComplete(rtFunctionRef& func) const
  {
    func = m_callback;
    return RT_OK;
  }
  rtError setOnUploadComplete(rtFunctionRef const& func)
  {
    m_callback = func;
    return RT_OK;
  }

  rtMethod2ArgAndReturn ("twoIntNumberSum", twoIntNumberSum, int32_t, int32_t, int32_t);
  rtError twoIntNumberSum(int32_t in1, int32_t in2, int32_t &res) {
    res = in2 + in2;
    rtLogDebug("At server side result : %d + %d = %d\n", in2, in2, res);
    return RT_OK;

 }

  // helper
  void fireOnUploadComplete()
  {
    if (m_callback)
    {
      rtString s("upload complete");
      rtError e = m_callback.send(s);
      if (e != RT_OK)
      {
        rtLogInfo("send:%s", rtStrError(e));
      }
    }
  }

private:
  rtString m_name;
  rtFunctionRef m_callback;
  int _n;
  rtObjectRef m_big;
};

// required stub definitions (they're macros)
rtDefineObject(ContinuousVideoRecorder, rtObject);
rtDefineProperty(ContinuousVideoRecorder, name);
rtDefineProperty(ContinuousVideoRecorder, onUploadComplete);
rtDefineProperty(ContinuousVideoRecorder, prop);
rtDefineProperty(ContinuousVideoRecorder, bigprop);
rtDefineMethod(ContinuousVideoRecorder, twoIntNumberSum);



int main(int argc, char* argv [])
{
  rtError e;
  rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
  e = rtRemoteInit(env);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteInit:%s", rtStrError(e));
    exit(1);
  }

  rtObjectRef obj(new ContinuousVideoRecorder());
  e = rtRemoteRegisterObject(env, "some_name", obj);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteRegisterObject:%s", rtStrError(e));
    exit(2);
  }

  
  while (true)
  {
    // process incoming messages
    e = rtRemoteRunUntil(env, 1000);
    rtValue v;
    obj->Get("prop", &v);
    printf("\nServer Side,  Value : %d\n",v.toInt32());
    
  }

  return 0;
}

