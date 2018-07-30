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

// pxWaylandContainer.cpp

#include <stdlib.h>
#include <unistd.h>
#include "rtString.h"
#include "rtRef.h"
#include "pxCore.h"
#include "pxKeycodes.h"

#include "pxWaylandContainer.h"

#include "pxContext.h"

#include <map>
using namespace std;

extern map<string, string> gWaylandAppsMap;
// // TODO: move this to pxOffscreenNative.{cpp,mm} files
// int __pxMain(int , char*[]) {return 0;}
// int pxMain(int , char*[]) __attribute__ ((weak, alias ("_Z8__pxMainiPPc")));

// #define MAX_FIND_REMOTE_TIMEOUT_IN_MS 5000
// #define FIND_REMOTE_ATTEMPT_TIMEOUT_IN_MS 100
// #define TEST_REMOTE_OBJECT_NAME "waylandClient123" //TODO - update

// #define UNUSED_PARAM(x) ((void)x)

pxWaylandContainer::pxWaylandContainer(pxScene2d* scene)
   : pxViewContainer(scene), mWayland(NULL),mClientPID(0),mFillColor(0),mHasApi(false),mBinary()
{
  addListener("onClientStarted", get<rtFunctionRef>("onClientStarted"));
  addListener("onClientStopped", get<rtFunctionRef>("onClientStopped"));
  addListener("onClientConnected", get<rtFunctionRef>("onClientConnected"));
  addListener("onClientDisconnected", get<rtFunctionRef>("onClientDisconnected"));
  mRemoteReady = new rtPromise();
}

pxWaylandContainer::~pxWaylandContainer()
{
  if ( mWayland )
  {
     mWayland->setEvents(NULL);
  }
  mRemoteReady = NULL;
  mWayland = NULL;
}

void pxWaylandContainer::dispose(bool pumpJavascript)
{
   setView(NULL);
   pxObject::dispose(pumpJavascript);
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
  size_t regcmdlen;
  const char *regcmd;
  const char *p= strpbrk( s, " ");
  std::map<string, string>::iterator it= gWaylandAppsMap.end();
  rtString binary;
  mCmd = s;
  if ( !p )
  {
    // Requested cmd has no args - use it as registry key
    it  = gWaylandAppsMap.find(s);
    if (it != gWaylandAppsMap.end())
    {
      regcmd= it->second.c_str();
      regcmdlen= strlen(regcmd);
      if ( regcmd[regcmdlen-1] == '%' )
      {
         // If matched registry item is marked as allowing args
         // remove the '%' suffix.
         regcmdlen -= 1;
      }
      if ( regcmdlen > 0 )
      {
         binary = rtString(regcmd, (uint32_t) regcmdlen);
      }
    }
  }
  else
  {
    // Requested cmd has args - use the cmd without args as a registry key
    const char *cmd= strndup( s, (p-s) );
    if ( cmd )
    {
       it  = gWaylandAppsMap.find(cmd);
       if (it != gWaylandAppsMap.end())
       {
          // If the registry entry permits arguments (has a '%' suffix) then form
          // the actual command to use from the registry entry and
          // the supplied arguments.
          const char *args= p;
          regcmd= it->second.c_str();
          regcmdlen= strlen(regcmd);
          if ( (strlen( args ) > 0) &&
               (regcmdlen > 1) &&
               (regcmd[regcmdlen-1] == '%'))
          {
             binary = rtString(regcmd, (uint32_t) (regcmdlen-1) );
             binary.append( args );
          }
       }
       free( (void*)cmd );
    }
  }
  if (it == gWaylandAppsMap.end())
  {
    rtLogError("Unrecognised wayland app \"%s\". please verify the app name or add entry in waylandregistry.conf \n",s);
    return RT_ERROR;
  }
  if ( mWayland )
  {
     mWayland->setCmd(binary);
  }
  mBinary = binary;
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
      rtPromise* remoteReady = (rtPromise*) mRemoteReady.getPtr();
      if (NULL != remoteReady)
        remoteReady->send(ready?"resolve":"reject",this);
  }
}

void pxWaylandContainer::sendPromise()
{
  if(mInitialized && !((rtPromise*)mReady.getPtr())->status())
  {
    int32_t processNameIndex = mBinary.find(0, ' ');
    rtString processName;
    if (processNameIndex > 0)
    {
      processName = mBinary.substring(0, processNameIndex);
    }
    else
    {
      processName = mBinary;
    }
    rtLogInfo("process name: %s orig: %s", processName.cString(), mBinary.cString());
    if (access( processName.cString(), F_OK ) != -1)
    {
      rtLogDebug("sending resolve promise");
      mReady.send("resolve",this);
    }
    else
    {
      rtLogDebug("sending reject promise");
      mReady.send("reject",this);
    }
  }
}

rtError pxWaylandContainer::suspend(const rtValue &v, bool& b)
{
  b = false;
  if ( mWayland )
  {
    mWayland->suspend(v);
    b = true;
  }
  return RT_OK;
}

rtError pxWaylandContainer::resume(const rtValue& v, bool& b)
{
  b = false;
  if ( mWayland )
  {
    mWayland->resume(v);
    b = true;
  }
  return RT_OK;
}

rtError pxWaylandContainer::destroy(bool& b)
{
  b = false;
  if ( mWayland )
  {
    mWayland->setEvents(NULL);
    setView(NULL);
    b = true;
  }
  mRemoteReady = NULL;
  mWayland = NULL;
  return RT_OK;
}

void pxWaylandContainer::onInit()
{
  if ( mWayland )
  {
    mWayland->setPos( mx, my );
    mWayland->useDispatchThread( true );
    mWayland->onInit();

    rtString name;
    if ( RT_OK == mWayland->displayName( name ) )
    {
       mDisplayName = name;
    }
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
rtDefineProperty(pxWaylandContainer,hasApi);
rtDefineMethod(pxWaylandContainer, suspend);
rtDefineMethod(pxWaylandContainer, resume);
rtDefineMethod(pxWaylandContainer, destroy);
