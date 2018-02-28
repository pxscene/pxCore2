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

rtError
onTickCallback(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  // this matches the signature of the server's function
  rtLogInfo("onTickCallback");
  rtLogInfo("argc:%d", argc);
  rtLogInfo("argv[0]:%s", argv[0].toString().cString());

  (void) result;
  (void) argp;

  return RT_OK;
}

/*
client-side:
1. find remote object by name
2. get/set properties from object
*/

int main(int /*argc*/, char* /*argv*/ [])
{
  rtLogSetLevel(RT_LOG_INFO);

  rtError e;
  rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
  e = rtRemoteInit(env);
  if (e != RT_OK) {
    rtLogError("rtRemoteInit:%s", rtStrError(e));
    exit(1);
  }

  // find object
  rtObjectRef obj;
  while ((e = rtRemoteLocateObject(env, "host_object", obj)) != RT_OK) {
    rtLogInfo("still looking:%s", rtStrError(e));
  }

  obj.set("count", 100);

  rtObjectRef objvar(new TestObject());
  obj.set("objvar", objvar);

  rtFunctionRef callback(new rtFunctionCallback(onTickCallback));
  obj.set("onTick", callback);

  while (true) {
    rtRemoteRunUntil(env, 1000);
    rtLogInfo("count=%d\n", obj.get<int>("count"));
  }
  return 0;
}
