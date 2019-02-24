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

// pxBrowserExample.cpp

#include "pxCore.h"
#include "pxTimer.h"
#include "pxEventLoop.h"
#include "pxWindow.h"

#define ANIMATION_ROTATE_XYZ

#include "pxContext.h"
#include "pxScene2d.h"
#include "pxBrowserView.h"


static pxBrowserView* view = nullptr;

pxEventLoop eventLoop;

pxContext context;


typedef rtRef <pxBrowserView> pxBrowserViewRef;


class pxBrowserViewContainer : public pxIViewContainer
{
public:
  pxBrowserViewContainer() : mRefCount(0), mBrowserView(NULL)
  {}

  virtual unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  virtual unsigned long Release()
  {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual void RT_STDCALL
  invalidateRect(pxRect
  * /*r*/) {}
  virtual void* RT_STDCALL

  getInterface(const char* /*t*/)
  { return NULL; }

  rtError setView(pxBrowserView* v)
  {
    mBrowserView = v;
    return RT_OK;
  }

  void onSize(int32_t w, int32_t h)
  {
    if (mBrowserView)
      mBrowserView->onSize(w, h);
  }

  void onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    if (mBrowserView)
      mBrowserView->onMouseDown(x, y, flags);
  }

  void onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    if (mBrowserView)
      mBrowserView->onMouseUp(x, y, flags);
  }

  void onMouseLeave()
  {
    if (mBrowserView)
      mBrowserView->onMouseLeave();
  }

  void onMouseMove(int32_t x, int32_t y)
  {
    if (mBrowserView)
      mBrowserView->onMouseMove(x, y);
  }

  void onScrollWheel(float dx, float dy)
  {
    if (mBrowserView)
      mBrowserView->onScrollWheel(dx, dy);
  }

  void onFocus()
  {
    if (mBrowserView)
      mBrowserView->onFocus();
  }

  void onBlur()
  {
    if (mBrowserView)
      mBrowserView->onBlur();
  }

  void onKeyDown(uint32_t keycode, uint32_t flags)
  {
    if (mBrowserView)
      mBrowserView->onKeyDown(keycode, flags);
  }

  void onKeyUp(uint32_t keycode, uint32_t flags)
  {
    if (mBrowserView)
      mBrowserView->onKeyUp(keycode, flags);
  }

  void onChar(uint32_t c)
  {
    if (mBrowserView)
      mBrowserView->onChar(c);
  }

  void onDraw()
  {
    if (mBrowserView)
      mBrowserView->onDraw();
  }

  void onUpdate(double t)
  {
    if (mBrowserView)
      mBrowserView->onUpdate(t);
  }

protected:
  rtAtomic mRefCount;
  pxBrowserViewRef mBrowserView;
};

typedef rtRef <pxBrowserViewContainer> pxBrowserViewContainerRef;


rtError
onConsoleLog(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  rtLogInfo("onConsoleLog start =========================");
  rtObjectRef logItem = argv[0].toObject();
  (void) result;
  (void) argp;
  rtValue v;

  logItem->Get("level", &v);
  rtLogInfo("level = %d", v.toInt32());

  logItem->Get("logMessage", &v);
  rtLogInfo("message = %s", v.toString().cString());

  logItem->Get("lineNumber", &v);
  rtLogInfo("lineNumber = %d", v.toInt32());

  logItem->Get("resourceId", &v);
  rtLogInfo("resourceId = %s", v.toString().cString());

  rtLogInfo("onConsoleLog end =========================");
  return RT_OK;
}

rtError onHTMLLinkClicked(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  (void) result;
  (void) argp;

  rtObjectRef item = argv[0].toObject();
  rtValue v;

  item->Get("value", &v);

  rtLogInfo("onHTMLLinkClicked, link = %s", v.toString().cString());
  return RT_OK;
}

rtError onHTMLDocumentLoaded(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  (void) result;
  (void) argp;

  rtObjectRef item = argv[0].toObject();
  rtValue v;


  item->Get("success", &v);

  rtLogInfo("onHTMLDocumentLoaded, success = %s", v.toBool() ? "true" : "false");

  view->dumpProperties();
  return RT_OK;
}

rtError onError(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  (void) result;
  (void) argp;

  rtObjectRef item = argv[0].toObject();
  rtValue v;

  item->Get("errorType", &v);
  rtLogInfo("onError errorType = %s", v.toString().cString());

  item->Get("description", &v);
  rtLogInfo("onError description = %s", v.toString().cString());

  return RT_OK;
}

rtError onCookieJarChanged(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  (void) result;
  (void) argp;

  rtObjectRef item = argv[0].toObject();
  rtValue v;

  item->Get("name", &v);
  rtLogInfo("onCookieJarChanged name = %s", v.toString().cString());

  item->Get("value", &v);
  rtLogInfo("onCookieJarChanged value = %s", v.toString().cString());

  item->Get("domain", &v);
  rtLogInfo("onCookieJarChanged domain = %s", v.toString().cString());

  return RT_OK;
}

class sceneWindow : public pxWindow, public pxIViewContainer
{
public:
  sceneWindow() : mWidth(0), mHeight(0), mBrowserViewContainer(NULL)
  {}

  virtual ~sceneWindow()
  {
    mBrowserViewContainer = NULL;
  }

  void init(int x, int y, int w, int h)
  {
    pxWindow::init(x, y, w, h);
    mWidth = w;
    mHeight = h;

    mBrowserViewContainer = new pxBrowserViewContainer();

#ifdef WIN32
    view = new pxBrowserView(&mWindow, w, h);
#elif __APPLE__
    view = new pxBrowserView(mWindow, w, h);
#endif
    mBrowserViewContainer->setView(view);
    mBrowserViewContainer->onSize(w, h);
  }

  void* getInterface(const char* /*name*/)
  {
    return NULL;
  }

  virtual void invalidateRect(pxRect* r)
  {
    pxWindow::invalidateRect(r);
  }

  void close()
  {
    onCloseRequest();
  }

protected:

  virtual void onSize(int32_t w, int32_t h)
  {
    if (mWidth != w || mHeight != h)
    {
      mWidth = w;
      mHeight = h;
      if (mBrowserViewContainer)
        mBrowserViewContainer->onSize(w, h);
    }
  }

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onMouseDown(x, y, flags);
  }

  virtual void onCloseRequest()
  {
    context.term();
    eventLoop.exit();
  }

  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onMouseUp(x, y, flags);
  }

  virtual void onMouseLeave()
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onMouseLeave();
  }

  virtual void onMouseMove(int32_t x, int32_t y)
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onMouseMove(x, y);
  }

  virtual void onScrollWheel(float dx, float dy)
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onScrollWheel(dx, dy);
  }

  virtual void onFocus()
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onFocus();
  }

  virtual void onBlur()
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onBlur();
  }

  virtual void onKeyDown(uint32_t keycode, uint32_t flags)
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onKeyDown(keycode, flags);
  }

  virtual void onKeyUp(uint32_t keycode, uint32_t flags)
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onKeyUp(keycode, flags);
  }

  virtual void onChar(uint32_t c)
  {
    if (mBrowserViewContainer)
      mBrowserViewContainer->onChar(c);
  }

  virtual void onDraw(pxSurfaceNative)
  {
    context.setSize(mWidth, mHeight);
    context.clear(mWidth, mHeight);
    if (mBrowserViewContainer)
      mBrowserViewContainer->onDraw();
  }

  virtual void onAnimationTimer()
  {
    if (mBrowserViewContainer)
    {
      invalidateRect(nullptr);
      mBrowserViewContainer->onUpdate(pxSeconds());
    }
  }

protected:
  int32_t mWidth;
  int32_t mHeight;

  pxBrowserViewContainerRef mBrowserViewContainer;
};

void initQT(){

  view->initQT();

  view->setVisible(true);
  view->setTransparent(true);
  view->setLocalStorageEnabled(true);
  view->setConsoleLogEnabled(true);
  view->setUserAgent("custom userAgent from qt view");


  // headers
  rtMapObject* headers = new rtMapObject();
  rtValue headerValue("qt-web-value");
  headers->Set("header-qt", &headerValue);
  view->setHeaders(headers);


  //proxy test
  pxBrowserProxy* proxy = new pxBrowserProxy();
  proxy->setType(QNetworkProxy::HttpProxy);
  proxy->setHostname("127.0.0.1");
  proxy->setPort(1087);
  //view->setProxy(proxy);

  // cookies
  rtArrayObject* cookies = new rtArrayObject();
  rtMapObject* cookie = new rtMapObject();

  rtValue name("cookie-01");
  cookie->Set("name", &name);

  rtValue value("cookie-value");
  cookie->Set("value", &value);

  rtValue domain("127.0.0.1");
  cookie->Set("domain", &domain);

  rtValue c(cookie);
  cookies->Set((uint32_t) 0, &c);
  view->setCookieJar(cookies);

  // events api
  view->addListener("ConsoleLog", new rtFunctionCallback(onConsoleLog));
  view->addListener("HTMLLinkClicked", new rtFunctionCallback(onHTMLLinkClicked));
  view->addListener("HTMLDocumentLoaded", new rtFunctionCallback(onHTMLDocumentLoaded));
  view->addListener("Error", new rtFunctionCallback(onError));
  view->addListener("CookieJarChanged", new rtFunctionCallback(onCookieJarChanged));

  rtValue url("http://127.0.0.1:3000");

  view->Set("url", &url);

  view->dumpProperties();
}

sceneWindow win;

#define QT_GL_CONTEXT_INDEX 101

int pxMain(int argc, char* argv[])
{
  rtLogSetLevel(RT_LOG_DEBUG);
  win.init(10, 10, 1280, 720);
  win.setTitle("QT Browser example");
  win.setAnimationFPS(60);
  win.setVisibility(true);


  context.init();
  context.enableInternalContext(true, QT_GL_CONTEXT_INDEX);
  initQT();
  context.enableInternalContext(false, QT_GL_CONTEXT_INDEX);
  eventLoop.run();

  return 0;
}
