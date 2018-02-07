#include "rtRemote.h"
#include "rtObject.h"
#include <assert.h>

class HostObject : public rtObject
{
  rtDeclareObject(HostObject, rtObject);
public:
  HostObject() : m_count(0) {}

  rtProperty(count, getCount, setCount, int);
  rtProperty(objvar, getObjVar, setObjVar, rtObjectRef);
  rtProperty(onTick, getOnTickCallback, setOnTickCallback, rtFunctionRef);

  rtError getCount(int &c) const { c = m_count; return RT_OK; }
  rtError setCount(int c) { m_count = c; return RT_OK; }

  rtError getObjVar(rtObjectRef &obj) const {
    obj = m_obj;
    return RT_OK;
  }
  rtError setObjVar(const rtObjectRef &obj) {
    m_obj = obj;
    return RT_OK;
  }

  rtError getOnTickCallback(rtFunctionRef &func) const {
    func = m_callback;
    return RT_OK;
  }
  rtError setOnTickCallback(const rtFunctionRef &func) {
    m_callback = func;
    return RT_OK;
  }

  void think() {
    if (m_callback) {
      rtString s("server");
      rtError e = m_callback.send(s);
      if (e != RT_OK) {
        rtLogInfo("send:%s", rtStrError(e));
      }
    }

    m_count++;

    if (m_obj) {
        m_obj.set("count", m_count);
    }
  }

private:
  int m_count;
  rtFunctionRef m_callback;
  rtObjectRef m_obj;
};

rtDefineObject(HostObject, rtObject);
rtDefineProperty(HostObject, count);
rtDefineProperty(HostObject, objvar);
rtDefineProperty(HostObject, onTick);

/*
server-side:
1. create server host classes 
2. register them with rtRemoteRegisterObject() with its name as argument
3. invoke rtRemoteRunUntil() or rtRemoteRun()
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

  rtObjectRef obj(new HostObject());
  e = rtRemoteRegisterObject(env, "host_object", obj);
  if (e != RT_OK) {
    rtLogError("rtRemoteRegisterObject:%s", rtStrError(e));
    exit(2);
  }

  while (true) {
    e = rtRemoteRunUntil(env, 1000);
    ((HostObject *)obj.getPtr())->think();
  }

  return 0;
}
