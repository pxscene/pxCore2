// pxCore CopyRight 2007-2015 John Robinson
// pxWaylandContainer.cpp

#include "rtString.h"
#include "rtRef.h"
#include "pxCore.h"
#include "pxKeycodes.h"
#include "pxWindow.h"

#include "pxWaylandContainer.h"

#include "pxContext.h"

#include <map>
using namespace std;

// // TODO: move this to pxOffscreenNative.{cpp,mm} files
// int __pxMain(int , char*[]) {return 0;}
// int pxMain(int , char*[]) __attribute__ ((weak, alias ("_Z8__pxMainiPPc")));

// #define MAX_FIND_REMOTE_TIMEOUT_IN_MS 5000
// #define FIND_REMOTE_ATTEMPT_TIMEOUT_IN_MS 100
// #define TEST_REMOTE_OBJECT_NAME "waylandClient123" //TODO - update

// #define UNUSED_PARAM(x) ((void)x)

pxWaylandContainer::pxWaylandContainer(pxScene2d* scene)
   : pxViewContainer(scene), mWayland(NULL)
{
  addListener("onClientStarted", get<rtFunctionRef>("onClientStarted"));
  addListener("onClientStopped", get<rtFunctionRef>("onClientStopped"));
  addListener("onClientConnected", get<rtFunctionRef>("onClientConnected"));
  addListener("onClientDisconnected", get<rtFunctionRef>("onClientDisconnected"));
  mRemoteReady = new rtPromise();
}

pxWaylandContainer::~pxWaylandContainer()
{
  mWayland->setEvents(NULL);
  mRemoteReady = NULL;
  mWayland = NULL;
}

void pxWaylandContainer::invalidate( pxRect* r )
{   
   invalidateRect(r);
   mScene->mDirty = true;
}

void pxWaylandContainer::hidePointer( bool hide )
{
   mScene->hidePointer( hide );
}

void pxWaylandContainer::clientStarted( int pid )
{
   mClientPID= pid;
   
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientStarted");
   e.set("target", this);
   e.set("pid", pid );
   mEmit.send("onClientStarted", e);
}

void pxWaylandContainer::clientConnected( int pid )
{
   mClientPID= pid;
 
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientConnected");
   e.set("target", this);
   e.set("pid", pid );
   mEmit.send("onClientConnected", e);
}

void pxWaylandContainer::clientDisconnected( int pid )
{
   mClientPID= pid;
   
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientDisconnected");
   e.set("target", this);
   e.set("pid", pid );
   mEmit.send("onClientDisconnected", e);
}

void pxWaylandContainer::clientStoppedNormal( int pid, int exitCode )
{
   mClientPID= -1;
   
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientStopped");
   e.set("target", this);
   e.set("pid", pid );
   e.set("crashed", false );
   e.set("exitCode", exitCode );
   mEmit.send("onClientStopped", e);
}

void pxWaylandContainer::clientStoppedAbnormal( int pid, int signo )
{
   mClientPID= -1;
   
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientStopped");
   e.set("target", this);
   e.set("pid", pid );
   e.set("crashed", true );
   e.set("signo", signo );
   mEmit.send("onClientStopped", e);
}

void pxWaylandContainer::isReady( bool ready )
{
  mReady.send(ready?"resolve":"reject", this);
  if ( ready )
  {
    rtObjectRef e = new rtMapObject;
    e.set("name", "onReady");
    e.set("target", this);
    mEmit.send("onReady", e);
  }
}

rtError pxWaylandContainer::displayName(rtString& s) const { s = mDisplayName; return RT_OK; }
rtError pxWaylandContainer::setDisplayName(const char* s) 
{
  mDisplayName = s;
  if ( mWayland )
  {
     mWayland->setDisplayName(s);
  }
  return RT_OK;
}

rtError pxWaylandContainer::setCmd(const char* s)
{
  mCmd = s;
  if ( mWayland )
  {
     mWayland->setCmd(s);
  }
  return RT_OK;
}

rtError pxWaylandContainer::setRemoteServer(const char* s)
{
  mRemoteServer = s;
  if ( mWayland )
  {
     mWayland->setRemoteObjectName(s);
  }
  return RT_OK;
}

rtError pxWaylandContainer::fillColor(uint32_t& c) const 
{
  c= mFillColor;
  return RT_OK;
}

rtError pxWaylandContainer::setFillColor(uint32_t c) 
{
  mFillColor= c;
  if ( mWayland )
  {
     mWayland->setFillColor(c);
  }
  return RT_OK;
}

rtError pxWaylandContainer::hasApi(bool& v) const
{
  v=mHasApi;
  return RT_OK;
}

rtError pxWaylandContainer::setHasApi(bool v)
{
  mHasApi = v;
  if ( mWayland )
  {
     mWayland->setHasApi(v);
  }
  return RT_OK;
}

rtError pxWaylandContainer::api(rtValue& v) const
{
  if ( mWayland )
  {
     return mWayland->api(v);
  }
  return RT_PROP_NOT_FOUND;
}

rtError pxWaylandContainer::setView(pxWayland* v)
{
  if (mWayland && (mWayland != v) )
  {
     mWayland->setEvents(NULL);
  }
  mWayland= v;
  if ( mWayland )
  {
     mWayland->setPos( mx, my );
     mWayland->setEvents( this );
  }
  return pxViewContainer::setView(v);
}

rtError pxWaylandContainer::remoteReady(rtValue& promise) const
{
  if (NULL != mRemoteReady)
  {
      promise = mRemoteReady;
  }
  return RT_OK;
}

void pxWaylandContainer::isRemoteReady(bool ready)
{
  if (NULL != mRemoteReady)
  {
      mRemoteReady->send(ready?"resolve":"reject",this);
  }
}

void pxWaylandContainer::onInit()
{
  if ( mWayland )
  {
    mWayland->setPos( mx, my );
    mWayland->useDispatchThread( true );
    mWayland->onInit();
  }
}

rtDefineObject(pxWaylandContainer,pxViewContainer);
rtDefineProperty(pxWaylandContainer,displayName);
rtDefineProperty(pxWaylandContainer,cmd);
rtDefineProperty(pxWaylandContainer,clientPID);
rtDefineProperty(pxWaylandContainer,fillColor);
rtDefineProperty(pxWaylandContainer,api);
rtDefineProperty(pxWaylandContainer,remoteReady);
rtDefineProperty(pxWaylandContainer,server);
