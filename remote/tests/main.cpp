#include <glib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <unistd.h>
#include <rtRemote.h>

class rtTest : public rtObject {
public:
    rtDeclareObject(rtTest, rtObject);
    rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);

    rtTest() : m_emit(new rtEmit()) {
        g_timeout_add_seconds(1, [](gpointer data) -> gboolean {
            rtTest& test = *static_cast<rtTest*>(data);
            rtError rc = test.sendEvent();
            return rc == RT_OK ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
        }, this);
    }
    rtError sendEvent() {
        rtObjectRef e = new rtMapObject;
        e.set("name", "onTestEvent");
        return m_emit.send("onTestEvent", e);
    }
    rtError addListener(rtString eventName, const rtFunctionRef& f) {
        return m_emit->addListener(eventName, f);
    }
private:
    rtEmitRef m_emit;
};

rtDefineObject(rtTest, rtObject);
rtDefineMethod(rtTest, addListener);

static char const* objectName = "rtTest";

void
RunServer()
{
  rtLogInfo("running server");
  rtObjectRef objectRef = new rtTest();
  rtError rc = rtRemoteRegisterObject(objectName, objectRef);
  assert(rc == RT_OK);
}


void
RunClient()
{
  rtLogInfo("running client");
  rtError rc;
  rtObjectRef objectRef;

  do 
  {
    rc = rtRemoteLocateObject(objectName, objectRef);
  }
  while (rc != RT_OK);
  assert(rc == RT_OK);

  rtLogInfo("found object: %s", objectName);

  rc = objectRef.send("on", "onTestEvent", new rtFunctionCallback(
        [](int numArgs, const rtValue* args, rtValue* result, void* context) -> rtError 
        {
          rtLogInfo("numArgs: %d", numArgs);
          if (numArgs == 1) 
          {
            rtObjectRef event = args[0].toObject();
            rtString eventName = event.get<rtString>("name");
            rtLogInfo("Received event: %s", eventName.cString());
          }
          else
          {
            rtLogError("*** Error: received unknown event");
          }
          if (result)
            *result = rtValue(true);
          return RT_OK;
        },
        NULL));

  rtLogInfo("send: %d", rc);

  assert(rc == RT_OK);
}

int main(int argc, char *argv[])
{
  rtError rc;
  rc = rtRemoteInit();
  assert(rc == RT_OK);

  if (argc > 1)
    RunClient();
  else
    RunServer();

  GMainLoop *loop = g_main_loop_new(NULL, FALSE);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  rtRemoteShutdown();
  return 0;
}
