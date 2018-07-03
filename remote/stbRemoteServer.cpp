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


class DisplayManager : public rtObject
{
  rtDeclareObject(DisplayManager, rtObject);
public:
  DisplayManager() : m_resolutionW(1280), m_resolutionH(720)
  {
  }

  // define resolution properties
  rtProperty(resolutionW, getResolutionW, setResolutionW, int32_t);

  rtProperty(resolutionH, getResolutionH, setResolutionH, int32_t);


  rtError getResolutionW(int32_t& w) const
  {
    w = m_resolutionW;
    return RT_OK;
  }

  rtError setResolutionW(int32_t w)
  {
    if (w <= 0)
    {
      return RT_ERROR_INVALID_ARG;
    }
    m_resolutionW = w;
    return RT_OK;
  }

  rtError getResolutionH(int32_t& h) const
  {
    h = m_resolutionH;
    return RT_OK;
  }

  rtError setResolutionH(int32_t h)
  {
    if (h <= 0)
    {
      return RT_ERROR_INVALID_ARG;
    }
    m_resolutionH = h;
    return RT_OK;
  }

  // define methods
  rtMethod2ArgAndNoReturn("setResolution", setResolution, int32_t, int32_t);

  rtMethodNoArgAndReturn("getResolution", getResolution, rtString);


  rtError setResolution(int32_t w, int32_t h)
  {
    rtLogInfo("setResolution invoked, w = %d, h = %d", w, h);
    rtError e = setResolutionW(w);
    if (e != RT_OK)
    {
      return e;
    }
    e = setResolutionH(h);
    if (e != RT_OK)
    {
      return e;
    }
    return RT_OK;
  }

  rtError getResolution(rtString& out)
  {
    char jsonBuff[128];
    sprintf(jsonBuff, "{\"w\":%d,\"h\":%d}", m_resolutionW, m_resolutionH);
    rtLogInfo("getResolution invoked, result = %s", jsonBuff);
    out = rtString(jsonBuff);
    return RT_OK;
  }

private:
  int32_t m_resolutionW;
  int32_t m_resolutionH;
};


rtDefineObject(DisplayManager, rtObject);

// ---- export properties
rtDefineProperty(DisplayManager, resolutionW);
rtDefineProperty(DisplayManager, resolutionH);

// ---- export methods
rtDefineMethod(DisplayManager, getResolution);
rtDefineMethod(DisplayManager, setResolution);

int
main(int /*argc*/, char* /*argv*/ [])
{
  rtLogSetLevel(RT_LOG_DEBUG);   // set log level
  rtError e;
  rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
  e = rtRemoteInit(env);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteInit:%s", rtStrError(e));
    exit(1);
  }

  rtObjectRef dmObj(new DisplayManager());

  // register 4 objects
  e = rtRemoteRegisterObject(env, "DisplayManager", dmObj);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteRegisterObject stb failed, error = %s", rtStrError(e));
    exit(2);
  }

  while (true)
  {
    e = rtRemoteRunUntil(env, 1000);
    if (e != RT_OK && e != RT_ERROR_QUEUE_EMPTY)
    {
      rtLogError("Error happened on main loop, error = %s, program will exit", rtStrError(e));
      break;
    }
  }
  return 0;
}
