// pxCore CopyRight 2007-2015 John Robinson
// pxWayland.h

#ifndef PX_WAYLAND_H
#define PX_WAYLAND_H

#include <pthread.h>
#include "pxContext.h"
#include "rtMutex.h"
#include "pxTexture.h"

#include "westeros-compositor.h"

class pxWayland: public pxObject {
public:
  rtDeclareObject(pxWayland, pxObject);
  rtProperty(displayName, displayName, setDisplayName, rtString);
  rtProperty(cmd, cmd, setCmd, rtString);
  rtReadOnlyProperty(clientPID, clientPID, int32_t);
  rtProperty(fillColor, fillColor, setFillColor, uint32_t);
  
  pxWayland(pxScene2d* scene);

  virtual ~pxWayland();

  virtual void onInit();
  
  rtError displayName(rtString& s) const;
  rtError setDisplayName(const char* s);

  rtError cmd(rtString& s) const { s = m_cmd; return RT_OK; }
  rtError setCmd(const char* s)
  {
     m_cmd = s;
     return RT_OK;
  }

  rtError fillColor(uint32_t& /*c*/) const 
  {
    rtLogWarn("fillColor not implemented");
    return RT_OK;
  }

  rtError setFillColor(uint32_t c) 
  {
    m_fillColor[0] = (float)((c>>24)&0xff)/255.0f;
    m_fillColor[1] = (float)((c>>16)&0xff)/255.0f;
    m_fillColor[2] = (float)((c>>8)&0xff)/255.0f;
    m_fillColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  rtError clientPID(int32_t& pid) const { pid = m_clientPID; return RT_OK; }
  
  void onKeyDown(uint32_t keycode, uint32_t flags);
  void onKeyUp(uint32_t keycode, uint32_t flags);
  void onMouseEnter();
  void onMouseLeave();
  void onMouseMove( float x, float y );
  void onMouseDown();
  void onMouseUp();

private:
  pthread_t m_clientMonitorThreadId;
  pthread_mutex_t m_mutex;
  bool m_clientMonitorStarted;
  
  static void invalidate( WstCompositor *wctx, void *userData );
  static void hidePointer( WstCompositor *wctx, bool hide, void *userData );
  static void clientStatus( WstCompositor *wctx, int status, int pid, int detail, void *userData );
  void handleClientStarted( int pid );
  void handleClientStoppedNormal( int pid, int exitCode );
  void handleClientStoppedAbnormal( int pid, int signo );
  void handleClientConnected( int pid );
  void handleClientDisconnected( int pid );
  void launchClient();
  void launchAndMonitorClient();
  void terminateClient();
  static void *clientMonitorThread( void *data );
  uint32_t getModifiers( uint32_t flags );
  uint32_t linuxFromPX( uint32_t keyCode );
      
protected:
  virtual void draw();
  void createDisplay(rtString displayName);
  
  rtString m_DisplayName;
  rtString m_cmd;
  int32_t m_clientPID;
  pxContextFramebufferRef m_fbo;
  WstCompositor *m_wctx;
  float m_fillColor[4];
};

#endif
