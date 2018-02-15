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

// pxWayland.h

#ifndef PX_WAYLAND_H
#define PX_WAYLAND_H

#include <atomic>
#include <pthread.h>
#include "pxIView.h"
#include "pxScene2d.h"
#include "pxContext.h"
#include "rtMutex.h"
#include "pxTexture.h"
#include "rtObject.h"
#ifdef ENABLE_PX_WAYLAND_RPC
#include "rtRemote.h"
#endif //ENABLE_PX_WAYLAND_RPC

#include "westeros-compositor.h"

class pxWaylandEvents {
public:
  pxWaylandEvents() {}
  virtual ~pxWaylandEvents() {}

  virtual void invalidate( pxRect* /*r*/ ) {}
  virtual void hidePointer( bool /*hide*/ ) {}
  virtual void clientStarted( int /*pid*/ ) {}  
  virtual void clientConnected( int /*pid*/ ) {}
  virtual void clientDisconnected( int /*pid*/ ) {}
  virtual void clientStoppedNormal( int /*pid*/, int /*exitCode*/ ) {}
  virtual void clientStoppedAbnormal( int /*pid*/, int /*signo*/ ) {}
  virtual void isReady( bool /*ready*/ ) {}
  virtual void isRemoteReady( bool /*ready*/ ) {}
  virtual void remoteDisconnected(void * /*data*/ ) {}
};

class pxWayland: public pxIView {

public:
  pxWayland(bool usefbo=false);
  virtual ~pxWayland();

  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual void setViewContainer(pxIViewContainer* l) 
  {
    mContainer = l;
  }
  
  virtual void RT_STDCALL onCloseRequest()
  {
    rtLogInfo("pxWayland::onCloseRequest()");
  }

  void setEvents( pxWaylandEvents *events )
  {
     mEvents= events;
  }

  rtError displayName(rtString& s) const;
  rtError setDisplayName(const char* s);

  rtError cmd(rtString& s) const { s = mCmd; return RT_OK; }
  rtError setCmd(const char* s)
  {
     mCmd= s;
     return RT_OK;
  }

  rtError remoteObjectName(rtString& s) const { s = mRemoteObjectName; return RT_OK; }
  rtError setRemoteObjectName(const char* s) {
    mRemoteObjectName = s;
    return RT_OK;
  }

  rtError setFillColor(uint32_t c) 
  {
    mFillColor[0] = (float)((c>>24)&0xff)/255.0f;
    mFillColor[1] = (float)((c>>16)&0xff)/255.0f;
    mFillColor[2] = (float)((c>>8)&0xff)/255.0f;
    mFillColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  rtError hasApi(bool& v) const
  {
    v=mHasApi;
    return RT_OK;
  }
  rtError setHasApi(bool v)
  {
    mHasApi = v;
    return RT_OK;
  }
  
  rtError api(rtValue& v) const
  {
    mRemoteObjectMutex.lock();
    v = mAPI;
    mRemoteObjectMutex.unlock();
    return RT_OK;
  }
  
  void setPos( int x, int y )
  {
     mX= x;
     mY= y;
  }

  virtual void onInit();

  virtual void onSize(int32_t w, int32_t h);
  virtual bool onMouseDown(int32_t x, int32_t y, uint32_t flags);
  virtual bool onMouseUp(int32_t x, int32_t y, uint32_t flags);
  virtual bool onMouseEnter();
  virtual bool onMouseLeave();
  virtual bool onMouseMove(int32_t x, int32_t y);
  virtual bool onFocus();
  virtual bool onBlur();
  virtual bool onKeyDown(uint32_t keycode, uint32_t flags);
  virtual bool onKeyUp(uint32_t keycode, uint32_t flags);
  virtual bool onChar(uint32_t codepoint);
  virtual void onUpdate(double t);
  virtual void onDraw();

  rtError setProperty(const rtString &prop, const rtValue &val);
  rtError callMethod(const char* messageName, int numArgs, const rtValue* args);
  rtError addListener(const rtString& eventName, const rtFunctionRef& f);
  rtError delListener(const rtString& eventName, const rtFunctionRef& f);
  rtError startRemoteObjectLocator();
  rtError connectToRemoteObject(unsigned int timeout_ms);
  rtError useDispatchThread(bool use);
  rtError resume();
  rtError suspend();
private:
  rtAtomic mRefCount;
  pthread_t mClientMonitorThreadId;
  pthread_t mFindRemoteThreadId;
  pxIViewContainer *mContainer;
  bool mReadyEmitted;
  bool mClientMonitorStarted;
  std::atomic<bool> mWaitingForRemoteObject;
  bool mUseDispatchThread;
  int mX;
  int mY;
  int mWidth;
  int mHeight;
  bool mUseFbo;
  bool mSuspended;
  pxMatrix4f mLastMatrix;

  static void invalidate( WstCompositor *wctx, void *userData );
  static void hidePointer( WstCompositor *wctx, bool hide, void *userData );
  static void clientStatus( WstCompositor *wctx, int status, int pid, int detail, void *userData );
  static void remoteDisconnectedCB(void *data);

  void handleInvalidate();
  void handleHidePointer( bool hide );
  void handleClientStatus( int status, int pid, int detail );
  void launchClient();
  void launchAndMonitorClient();
  void terminateClient();
  static void *clientMonitorThread( void *data );
  static void *findRemoteThread(void *data);
  uint32_t getModifiers( uint32_t flags );
  bool isRotated();
  uint32_t linuxFromPX( uint32_t keyCode );
  void startRemoteObjectDetection();
  rtError connectToRemoteObject();

protected:
  void createDisplay(rtString displayName);

  rtString mDisplayName;
  rtString mCmd;
  pxWaylandEvents *mEvents;
  int32_t mClientPID;
  pxContextFramebufferRef mFBO;
  WstCompositor *mWCtx;
  float mFillColor[4];
  float mClearColor[4];

  bool mHasApi;
  rtValue mAPI;
#ifdef ENABLE_PX_WAYLAND_RPC
  rtObjectRef mRemoteObject;
#endif //ENABLE_PX_WAYLAND_RPC
  rtString mRemoteObjectName;
  mutable rtMutex mRemoteObjectMutex;
};

typedef rtRef<pxWayland> pxWaylandRef;

#endif

