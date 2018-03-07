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

// pxWaylandContainer.h

#ifndef PX_WAYLAND_CONTAINER_H
#define PX_WAYLAND_CONTAINER_H

#include "pxWayland.h"

class pxWaylandContainer: public pxViewContainer, pxWaylandEvents {
  rtDeclareObject(pxWaylandContainer, pxViewContainer);
  rtProperty(displayName, displayName, setDisplayName, rtString);
  rtProperty(cmd, cmd, setCmd, rtString);
  rtProperty(server, remoteServer, setRemoteServer, rtString);
  rtReadOnlyProperty(clientPID, clientPID, int32_t);
  rtProperty(fillColor, fillColor, setFillColor, uint32_t);
  rtProperty(hasApi, hasApi, setHasApi, bool);
  rtReadOnlyProperty(api, api, rtValue);
  rtReadOnlyProperty(remoteReady, remoteReady, rtValue);
  rtMethodNoArgAndReturn("suspend", suspend, bool);
  rtMethodNoArgAndReturn("resume", resume, bool);
  rtMethodNoArgAndReturn("destroy", destroy, bool);
public:
  pxWaylandContainer(pxScene2d* scene);
  ~pxWaylandContainer();
  rtError setView(pxWayland* v);
  virtual void dispose(bool pumpForChild=true, bool isRoot=false);

  virtual void onInit();

  virtual void invalidate( pxRect* r );
  virtual void hidePointer( bool hide );
  virtual void clientStarted( int pid );
  virtual void clientConnected( int pid );
  virtual void clientDisconnected( int pid );
  virtual void clientStoppedNormal( int pid, int exitCode );
  virtual void clientStoppedAbnormal( int pid, int signo );
  virtual void isReady( bool ready );
  virtual void isRemoteReady(bool ready);
  virtual void sendPromise();

  rtError displayName(rtString& s) const;
  rtError setDisplayName(const char* s);

  rtError cmd(rtString& s) const { s = mCmd; return RT_OK; }
  rtError setCmd(const char* s);

  rtError remoteServer(rtString& s) const { s = mRemoteServer; return RT_OK; }
  rtError setRemoteServer(const char* s);

  rtError clientPID(int32_t& pid) const { pid = mClientPID; return RT_OK; }

  rtError fillColor(uint32_t& c) const;
  rtError setFillColor(uint32_t c);

  rtError hasApi(bool& v) const;
  rtError setHasApi(bool v);

  rtError api(rtValue& v) const;
  rtError remoteReady(rtValue& v) const;

  rtError suspend(bool& b);
  rtError resume(bool& b);
  rtError destroy(bool& b);

private:
  rtString mDisplayName;
  rtString mCmd;
  rtString mRemoteServer;
  pxWaylandRef mWayland;
  int32_t mClientPID;
  uint32_t mFillColor;
  bool mHasApi;
  rtValue mAPI;  
  rtObjectRef mRemoteReady;
  rtString mBinary;
};

typedef rtRef<pxWayland> pxWaylandRef;

#endif

