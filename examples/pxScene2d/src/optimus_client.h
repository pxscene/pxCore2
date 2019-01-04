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
    rtEmitRef mEmit;

  private:
    rtObjectRef mRemoteObject;
    rtObjectRef mTestObj;
  };
}

#endif //RT_OPTIMUS_H
