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
#include <thread>
#include <unistd.h>
#include <assert.h>


class TestObject : public rtObject
{
  rtDeclareObject(TestObject, rtObject);
public:
  rtProperty(count, getCount, setCount, int);
  rtError getCount(int& n) const { n = m_count; return RT_OK; }
  rtError setCount(int  n) { m_count = n; return RT_OK; }
private:
  int m_count;
};

rtDefineObject(TestObject, rtObject);
rtDefineProperty(TestObject, count);

rtObjectRef obj;

void run()
{
  int n = 0;
  rtError e;
  rtValue val;

  while (true)
  {
    val = n++;

    e = obj->Set("prop", &val);
    if (e != RT_OK)
    {
      rtLogInfo("Set:%s", rtStrError(e));
      assert(false);
    }
    usleep(1000 * 100);

    e = obj->Get("prop", &val);
    if (e != RT_OK)
    {
      rtLogInfo("Get:%s - %d", rtStrError(e), val.toInt32());
      assert(false);
    }
  }
}

rtError
upload_complete(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  // this matches the signature of the server's function
  rtLogInfo("upload_complete");
  rtLogInfo("argc:%d", argc);
  rtLogInfo("argv[0]:%s", argv[0].toString().cString());

  (void) result;
  (void) argp;

  return RT_OK;
}


int main(int /*argc*/, char* /*argv*/ [])
{
  rtError e;
  rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
  e = rtRemoteInit(env);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteInit:%s", rtStrError(e));
    exit(1);
  }

  // find object
  while ((e = rtRemoteLocateObject(env, "some_name", obj)) != RT_OK)
  {
    rtLogInfo("still looking:%s", rtStrError(e));
  }

  rtFunctionRef callback(new rtFunctionCallback(upload_complete));
  obj.set("onUploadComplete", callback);

  rtObjectRef big(new TestObject());
  obj.set("bigprop", big);

  // std::thread t(run);

  int n = 10;
  while (true)
  {
    rtValue val;
    #if 0
    e = obj->Get("onUploadComplete", &val);
    rtLogInfo("Get:%s", rtStrError(e));
    rtLogInfo("Type:%s", val.getTypeStr());
    rtLogInfo("Addr:%p", val.toFunction().getPtr());
    #endif
    e = obj->Get("prop", &val);
    rtLogInfo("get  :%s", rtStrError(e));
    rtLogInfo("type :%s", val.getTypeStr());
    rtLogInfo("value:%d", val.toInt32());
    rtRemoteRunUntil(env, 100, true);
    //sleep(1);

    rtValue temp(n);
    obj->Set("prop", &temp);
    n++;
  }
  return 0;
}
