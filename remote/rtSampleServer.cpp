/*
 * Copyright [2017] [Comcast, Corp.]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "rtRemote.h"
#include "rtObject.h"

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

  // properties can be functions too!
  rtProperty(onUploadComplete, getOnUploadComplete, setOnUploadComplete, rtFunctionRef);
  rtError getOnUploadComplete(rtFunctionRef& func) const
  {
    func = m_callback;
  }
  rtError setOnUploadComplete(rtFunctionRef const& func)
  {
    m_callback = func;
  }

  // helper
  void fireOnUploadComplete()
  {
    if (m_callback)
    {
      rtString s("upload complete");
      rtError e = m_callback.send(s);
      rtLogInfo("send:%s", rtStrError(e));
    }
  }

private:
  rtString m_name;
  rtFunctionRef m_callback;
};

// required stub definitions (they're macros)
rtDefineObject(ContinuousVideoRecorder, rtObject);
rtDefineProperty(ContinuousVideoRecorder, name);
rtDefineProperty(ContinuousVideoRecorder, onUploadComplete);

int main(int /*argc*/, char* /*argv*/ [])
{
  rtLogSetLevel(RT_LOG_INFO);

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
    rtLogInfo("rtRemoteRunUntil: %s", rtStrError(e));

    ((ContinuousVideoRecorder *)obj.getPtr())->fireOnUploadComplete();
  }

  return 0;
}
