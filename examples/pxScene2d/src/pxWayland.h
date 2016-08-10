// pxCore CopyRight 2007-2015 John Robinson
// pxWayland.h

#ifndef PX_WAYLAND_H
#define PX_WAYLAND_H

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
};

class pxWayland: public pxIView {

public:
  pxWayland();
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

private:
  rtAtomic mRefCount;
  pthread_t mClientMonitorThreadId;
  pthread_t mFindRemoteThreadId;
  pxIViewContainer *mContainer;
  bool mReadyEmitted;
  bool mClientMonitorStarted;
  bool mWaitingForRemoteObject;
  int mX;
  int mY;
  int mWidth;
  int mHeight;

  static void invalidate( WstCompositor *wctx, void *userData );
  static void hidePointer( WstCompositor *wctx, bool hide, void *userData );
  static void clientStatus( WstCompositor *wctx, int status, int pid, int detail, void *userData );

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
  rtError startRemoteObjectLocator();

protected:
  void createDisplay(rtString displayName);

  rtString mDisplayName;
  rtString mCmd;
  pxWaylandEvents *mEvents;
  int32_t mClientPID;
  pxContextFramebufferRef mFBO;
  WstCompositor *mWCtx;
  float mFillColor[4];

  bool mHasApi;
  rtValue mAPI;
#ifdef ENABLE_PX_WAYLAND_RPC
  rtObjectRef mRemoteObject;
#endif //ENABLE_PX_WAYLAND_RPC
  rtString mRemoteObjectName;
  mutable rtMutex mRemoteObjectMutex;
};

class pxWaylandContainer: public pxViewContainer, pxWaylandEvents {
  rtDeclareObject(pxWaylandContainer, pxViewContainer);
  rtProperty(displayName, displayName, setDisplayName, rtString);
  rtProperty(cmd, cmd, setCmd, rtString);
  rtReadOnlyProperty(clientPID, clientPID, int32_t);
  rtProperty(fillColor, fillColor, setFillColor, uint32_t);
  rtProperty(hasApi, hasApi, setHasApi, bool);
  rtReadOnlyProperty(api, api, rtValue);

public:
  pxWaylandContainer(pxScene2d* scene);

  rtError setView(pxWayland* v);

  virtual void onInit();

  virtual void invalidate( pxRect* r );
  virtual void hidePointer( bool hide );
  virtual void clientStarted( int pid );
  virtual void clientConnected( int pid );
  virtual void clientDisconnected( int pid );
  virtual void clientStoppedNormal( int pid, int exitCode );
  virtual void clientStoppedAbnormal( int pid, int signo );
  virtual void isReady( bool ready );

  rtError displayName(rtString& s) const;
  rtError setDisplayName(const char* s);

  rtError cmd(rtString& s) const { s = mCmd; return RT_OK; }
  rtError setCmd(const char* s);

  rtError clientPID(int32_t& pid) const { pid = mClientPID; return RT_OK; }

  rtError fillColor(uint32_t& c) const;
  rtError setFillColor(uint32_t c);

  rtError hasApi(bool& v) const;
  rtError setHasApi(bool v);

  rtError api(rtValue& v) const;

private:
  rtString mDisplayName;
  rtString mCmd;
  pxWayland *mWayland;
  int32_t mClientPID;
  uint32_t mFillColor;
  bool mHasApi;
  rtValue mAPI;  
};

typedef rtRefT<pxWayland> pxWaylandRef;

#endif

