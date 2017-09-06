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

rtError
upload_complete(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  // this matches the signature of the server's function
  rtLogInfo("upload_complete");
  rtLogInfo("argc:%d", argc);
  rtLogInfo("argv[0]:%s", argv[0].toString().cString());
  return RT_OK;
}


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

  // find object
  rtObjectRef obj;
  while ((e = rtRemoteLocateObject(env, "some_name", obj)) != RT_OK)
  {
    rtLogInfo("still looking:%s", rtStrError(e));
  }

  obj.set("onUploadComplete", new rtFunctionCallback(upload_complete));

  while (true)
  {
    e = rtRemoteRunUntil(env, 30000);
    rtLogInfo("rtRemoteRunUntil:%s", rtStrError(e));
  }
  return 0;
}
