#ifndef RT_OPTIMUS_H
#define RT_OPTIMUS_H

#include "rtObject.h"
#include <queue>
#include <string>

namespace OptimusClient
{
  rtError pumpRemoteObjectQueue();
  rtError registerApi(rtObjectRef object);
  rtError getService(rtString name, rtObjectRef &returnObject);
  class rtOptimus : public rtObject
  {
  public:
    rtOptimus();

    ~rtOptimus();

    rtDeclareObject(rtOptimus, rtObject);

    rtProperty(sceneContainer, sceneContainer, setSceneContainer, rtValue);
    rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
    rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);

    // rtObject function handlers
    virtual rtError addListener(rtString eventName, const rtFunctionRef &f);
    virtual rtError delListener(rtString eventName, const rtFunctionRef &f);
    rtError sceneContainer(rtValue& v) const;
    rtError setSceneContainer(const rtValue& v);

  protected:
    rtError shutdown();

  private:
    rtEmitRef mEmit;
    rtObjectRef mRemoteObject;
    rtObjectRef mTestObj;
  };
}

#endif //RT_OPTIMUS_H
