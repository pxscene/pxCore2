// pxCore CopyRight 2007-2015 John Robinson
// pxWaylandContainer.h

#ifndef PX_WAYLAND_CONTAINER_H
#define PX_WAYLAND_CONTAINER_H

#include "pxWayland.h"

class pxWaylandContainer: public pxViewContainer, pxWaylandEvents {
  rtDeclareObject(pxWaylandContainer, pxViewContainer);
  rtProperty(displayName, displayName, setDisplayName, rtString);
  rtProperty(cmd, cmd, setCmd, rtString);
  rtReadOnlyProperty(clientPID, clientPID, int32_t);
  rtProperty(fillColor, fillColor, setFillColor, uint32_t);
  rtProperty(hasApi, hasApi, setHasApi, bool);
  rtReadOnlyProperty(api, api, rtValue);
  rtReadOnlyProperty(remoteReady, remoteReady, rtValue);

public:
  pxWaylandContainer(pxScene2d* scene);
  ~pxWaylandContainer();
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
  virtual void isRemoteReady(bool ready);

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
  rtError remoteReady(rtValue& v) const;

private:
  rtString mDisplayName;
  rtString mCmd;
  pxWayland *mWayland;
  int32_t mClientPID;
  uint32_t mFillColor;
  bool mHasApi;
  rtValue mAPI;  
  rtPromise* mRemoteReady;
};

typedef rtRefT<pxWayland> pxWaylandRef;

#endif

