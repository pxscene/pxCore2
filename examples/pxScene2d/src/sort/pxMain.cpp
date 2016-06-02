// pxCore CopyRight 2007-2015 John Robinson
// main.cpp

#ifdef WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxViewWindow.h"

#include "rtLog.h"
#include "rtRefT.h"
#include "rtPathUtils.h"
#include "pxScene2d.h"

#include "testScene.h"

#include "pxFileDownloader.h"

//extern rtRefT<pxScene2d> scene;

pxEventLoop eventLoop;
class testWindow: public pxViewWindow
{
private:
  void onKeyDown(uint32_t keycode, uint32_t flags);
  void onCloseRequest();
};

bool flag = false;
testWindow win;


int pxMain()
{
  int width = 1280;
  int height = 720;

  win.init(10, 10, width, height);

  win.setTitle("pxCore!");
  win.setVisibility(true);

#if 1
  pxScene2dRef s = testScene();
//  pxScene2dRef s = new pxScene2d(true);
  printf("Before setView\n");
  win.setView(s);
  printf("Before setView(NULL)\n");
  win.setView(NULL);
#if 0
  {
  printf("pxObject should be destroyed\n");
  rtRefT<pxObject> o = new pxObject(s);
  }
#endif
  s = NULL;
#endif



  win.setAnimationFPS(60);

#if 0 
#ifndef __APPLE__
  testWindow win2;

  win2.init(110, 110, width, height);

  win2.setTitle("pxCore! 2");
  win2.setVisibility(true);
  win.setView(testScene());
  win2.setAnimationFPS(60);
#endif
#endif

  eventLoop.run();

  printf("after event loop");

  return 0;
}

void testWindow::onKeyDown(uint32_t keycode, uint32_t flags)
{
  pxViewWindow::onKeyDown(keycode, flags);
  printf("keycode %d\n", keycode);
  
  if (keycode == 65)
  {
    if (flag)
    {
      pxScene2dRef s = testScene();
      win.setView(s);
    }
    else
      win.setView(NULL);

    flag = !flag;
  }
}

void testWindow::onCloseRequest()
{
  printf("in onCloseRequest()");
  // When someone clicks the close box no policy is predefined.
  // so we need to explicitly tell the event loop to exit
  eventLoop.exit();
}  
