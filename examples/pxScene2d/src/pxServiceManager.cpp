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

#include "pxServiceManager.h"

rtRemoteEnvironment* pxServiceManager::mEnv = nullptr;

rtError pxServiceManager::findServiceManager(rtObjectRef &result)
{
  if (mEnv == nullptr)
  {
    mEnv = rtEnvironmentGetGlobal();
    rtError e = rtRemoteInit(mEnv);

    if (e != RT_OK)
    {
      rtLogInfo("rtRemoteInit e = %d", e);
      return e;
    }
  }

  rtError e = rtRemoteLocateObject(mEnv, SERVICE_MANAGER_OBJECT_NAME, result);

  if (e != RT_OK)
    rtLogInfo("rtRemoteLocateObject " SERVICE_MANAGER_OBJECT_NAME " e = %d\n", e);

  return e;
}
