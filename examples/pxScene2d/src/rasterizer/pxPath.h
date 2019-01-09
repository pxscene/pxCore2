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

#ifndef _PX_PATH_H
#define _PX_PATH_H

#ifdef _WINDOWS_
#define RTPLATFORM_WINDOWS
#endif


#include "pxContext.h"
#include "pxResource.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class pxPath: public pxObject
{
public:
  rtDeclareObject(pxPath, pxObject);
  
  rtProperty(d, path, setPath, rtString);

public:
  
  pxPath(pxScene2d* scene) : pxObject(scene)
  {
    mw = -1;
    mh = -1;
  };
  
  ~pxPath();

  virtual void draw();
  virtual void onInit();
  virtual void sendPromise();

  virtual rtError setPath(const rtString d);
  virtual rtError path(rtString& v) const { v = mPath; return RT_OK; };

  rtString     mPath;
  pxOffscreen  mImage;
  
  pxTextureRef mTexture;

}; // CLASS - pxPath



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //_PX_PATH_H
