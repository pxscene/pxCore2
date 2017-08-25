/*
 
 pxCore Copyright 2005-2017 John Robinson
 
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

#ifndef _PX_CANVAS_H
#define _PX_CANVAS_H

#ifdef _WINDOWS_
#define RTPLATFORM_WINDOWS
#endif


#include "pxCanvas2d.h"
#include "pxContext.h"
#include "pxResource.h"


class pxPath; //fwd

class pxCanvas: public pxObject
{
public:
  rtDeclareObject(pxCanvas, pxObject);

public:
   pxCanvas(pxScene2d* scene);
  ~pxCanvas();
  
  void draw();
    
  virtual void onInit();
  virtual void sendPromise();
  virtual void createNewPromise() { rtLogDebug("pxCanvas ignoring createNewPromise\n"); }

  
  void needsRedraw()  { mCanvasCtx.clear(); mCanvasCtx.needsRedraw(); };
  
  rtError drawPath(rtObjectRef path);

  rtMethodNoArgAndNoReturn("path", init);
  rtMethod1ArgAndNoReturn("drawPath", drawPath, rtObjectRef);

private:

  pxCanvas2d  mCanvasCtx;
};

#endif //_PX_CANVAS_H
