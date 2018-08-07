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

// testScene.cpp

#include "testScene.h"

#include "rtPathUtils.h"
#include "pxScene2d.h"
#include "pxText.h"
#include "pxTextBox.h"
#include "pxImage.h"
#include "pxKeycodes.h"
#include "pxInterpolators.h"

rtString bananaUrl;
rtString ballUrl;

rtError onSizeCB(int numArgs, const rtValue* args, rtValue* /*result*/, void* context)
{
  if (context)
  {
    pxScene2d* s = (pxScene2d*)context;

    rtObjectRef bg1;
    rtObjectRef bg2;
    s->getRoot()->getObjectById("bg1", bg1);
    s->getRoot()->getObjectById("bg2", bg2);
        
    if (numArgs == 1)
    {
      rtObjectRef e = args[0].toObject();
      int w = e.get<uint32_t>("w");
      int h = e.get<uint32_t>("h");
      bg1.set("w", w);
      bg1.set("h", h);
      bg2.set("w", w);
      bg2.set("h", h);
    }
  }
  return RT_OK;
}

rtError onKeyDownCB(int numArgs, const rtValue* args, rtValue* /*result*/, void* context)
{
  rtLogDebug("in keydowncb\n");
  if (context) 
  {
    pxScene2d* s = (pxScene2d*)context;

    rtObjectRef picture;
    s->getRoot()->getObjectById("picture",picture);
    
    if (numArgs>0)
    {
      rtObjectRef e = args[0].toObject();
      uint32_t keyCode = e.get<uint32_t>("keyCode");
      rtLogDebug("received keyCode %d\n", keyCode);
      switch(keyCode) 
      {
        // '1'
      case PX_KEY_ONE:
        rtLogInfo("banana\n");
        picture.set("url", bananaUrl);
        break;
        // '2'
      case PX_KEY_TWO:
        rtLogInfo("ball\n");
        picture.set("url", ballUrl);
        break;
      default:
        rtLogWarn("unhandled key");
        break;
      }
    }
  }
  return RT_OK;
}

pxScene2dRef testScene()
{
  rtObjectRef bg1;
  rtObjectRef bg2;
  rtObjectRef picture;

  pxScene2dRef scene = new pxScene2d;

  rtString d;
  rtGetCurrentDirectory(d);
  bananaUrl = d;
  ballUrl = d;
  bananaUrl.append("/../images/banana.png");
  ballUrl.append("/../images/ball.png");

  scene->init();

  rtObjectRef root = scene.get<rtObjectRef>("root");  

  root.send("on", "onKeyDown", new rtFunctionCallback(onKeyDownCB,
    scene.getPtr()));
  scene.send("on", "onResize", new rtFunctionCallback(onSizeCB,
    scene.getPtr()));

  rtString bgUrl;
  rtObjectRef props = new rtMapObject();
  props.set("t","image");  
  scene.sendReturns<rtObjectRef>("create", props, bg1);
  bgUrl = d;
  bgUrl.append("/../images/skulls.png");
  bg1.set("url", bgUrl);
  bg1.set("stretchX", 2);
  bg1.set("stretchY", 2);
  bg1.set("parent", root);
  bg1.set("w", scene->w());
  bg1.set("h", scene->h());
  bg1.set("id", "bg1");

  rtLogDebug("Try enumerating properties on image.\n");
  rtObjectRef keys = bg1.get<rtObjectRef>("allKeys");
  uint32_t length = keys.get<uint32_t>("length");
  for (uint32_t i = 0; i < length; i++)
  {
    rtLogDebug("i: %d key: %s\n", i, keys.get<rtString>(i).cString());
  }

  props = new rtMapObject();
  props.set("t","image");
  scene.sendReturns<rtObjectRef>("create", props, bg2);
  bgUrl = d;
  bgUrl.append("/../images/radial_gradient.png");
  bg2.set("url", bgUrl);
  bg2.set("stretchX", 1);
  bg2.set("stretchY", 1);
  bg2.set("parent", root);
  bg2.set("w", scene->w());
  bg2.set("h", scene->h());
  bg2.set("id", "bg2");

  rtObjectRef r;
  props = new rtMapObject();
  props.set("t","rect");
  scene.sendReturns<rtObjectRef>("create", props, r);
  if (r)
  {
    r.set("w", 300);
    r.set("h", 300);
    r.set("fillColor", 0xaaaaaaff);
    r.set("parent", root);
  }

  rtObjectRef t;
  props = new rtMapObject();
  props.set("t","text");
  scene.sendReturns<rtObjectRef>("create", props, t);

  t.set("text", "Select an image to display:\n\n"
        "1: Banana\n"
        "2: Ball");

  t.set("x", 10);
  t.set("y", 10);
  t.set("parent", root);

  props = new rtMapObject();
  props.set("t","image");
  scene.sendReturns<rtObjectRef>("create", props, picture);
  picture.set("x", 400);
  picture.set("y", 400);

  props = new rtMapObject();
  props.set("r",360.0);
  picture.send("animateTo", props, 0.5, pxConstantsAnimation::TWEEN_LINEAR, pxConstantsAnimation::OPTION_OSCILLATE,-1);
  picture.set("parent", root);
  picture.set("url", bananaUrl);
  picture.set("id","picture");

#if 1
  rtLogInfo("Enumerate children of root object\n");
  rtObjectRef c = root.get<rtObjectRef>("children");
  uint32_t l = c.get<uint32_t>("length");
#if 1
  for (uint32_t i = 0; i < l; i++)
  {
    rtObjectRef o = c.get<rtObjectRef>(i);
    rtString s;
    o.sendReturns<rtString>("description", s);
    rtLogInfo("class description: %s\n", s.cString());
  }
#endif
#endif
   return scene.getPtr();
}



