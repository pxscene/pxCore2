// pxCore CopyRight 2007-2015 John Robinson
// pxWayland.cpp

#include "rtString.h"
#include "rtRefT.h"
#include "pxCore.h"
#include "pxScene2d.h"
#include "pxKeycodes.h"
#include "pxWindow.h"

#include "pxWayland.h"

#include "pxContext.h"

extern "C"
{
#include "utf8.h"
}

#include <map>
using namespace std;

extern pxContext context;

#define MAX_FIND_REMOTE_TIMEOUT_IN_MS 5000
#define FIND_REMOTE_ATTEMPT_TIMEOUT_IN_MS 500
#define TEST_REMOTE_OBJECT_NAME "waylandClient123" //TODO - update

pxWayland::pxWayland(pxScene2d* scene) 
  : pxObject(scene), 
    m_clientMonitorThreadId(0), 
    m_findRemoteThreadId(0),
    m_clientMonitorStarted(false),
    m_waitingForRemoteObject(false),
    m_clientPID(-1),
    m_wctx(0),
    m_hasApi(false),
    m_API(),
    m_locator(),
    m_remoteObject(),
    m_remoteObjectName(TEST_REMOTE_OBJECT_NAME),
    m_remoteObjectMutex()
{
  pthread_mutex_init( &m_mutex, 0 );
  m_fillColor[0]= 0.0; 
  m_fillColor[1]= 0.0; 
  m_fillColor[2]= 0.0; 
  m_fillColor[3]= 0.0; 
  addListener("onClientStarted", get<rtFunctionRef>("onClientStarted"));
  addListener("onClientStopped", get<rtFunctionRef>("onClientStopped"));
  addListener("onClientConnected", get<rtFunctionRef>("onClientConnected"));
  addListener("onClientDisconnected", get<rtFunctionRef>("onClientDisconnected"));
}

pxWayland::~pxWayland()
{ 
  rtLogInfo("~pxWayland()"); 
  if ( m_wctx )
  {
     WstCompositorDestroy(m_wctx);
     
     terminateClient();
  } 
}

void pxWayland::onInit()
{
  setDisplayName(m_DisplayName);
}

rtError pxWayland::displayName(rtString& s) const { s = m_DisplayName; return RT_OK; }
rtError pxWayland::setDisplayName(const char* s) 
{
  m_DisplayName = s;
  if (mInitialized)
    createDisplay(m_DisplayName);
  else
    rtLogDebug("Deferring wayland display creation until pxWayland is initialized.");
  return RT_OK;
}

void pxWayland::sendPromise()
{
  if(mInitialized && !m_waitingForRemoteObject && !((rtPromise*)mReady.getPtr())->status())
  {
    mReady.send("resolve",this);
  }
}

void pxWayland::createDisplay(rtString displayName)
{
   bool error= false;
   const char* name = displayName.cString();
   const char* cmd = m_cmd.cString();

   rtLogInfo("pxWayland::createDisplay: %s\n", name);
   
   m_fbo= context.createFramebuffer( 0, 0 );
   
   m_wctx= WstCompositorCreate();
   if ( m_wctx )
   {
      if ( !WstCompositorSetIsEmbedded( m_wctx, true ) )
      {
         error= true;
         goto exit;
      }
      
      if ( !WstCompositorSetOutputSize( m_wctx, mw, mh ) )
      {
         error= true;
         goto exit;
      }
      
      if ( !WstCompositorSetInvalidateCallback( m_wctx, invalidate, this ) )
      {
         error= true;
         goto exit;
      }
      
      if ( !WstCompositorSetHidePointerCallback( m_wctx, hidePointer, this ) )
      {
         error= true;
         goto exit;
      }
      
      if ( !WstCompositorSetClientStatusCallback( m_wctx, clientStatus, this ) )
      {
         error= true;
         goto exit;
      }
      
      // If a display name was provided, use it.  Otherwise the
      // compositor will generate a unique display name to use.
      int len= (name ? strlen(name) : 0);
      if ( len > 0 )
      {
         if ( !WstCompositorSetDisplayName( m_wctx, name ) )
         {
            error= true;
            goto exit;
         }
      }
      
      if ( !WstCompositorStart( m_wctx ) )
      {
         error= true;
         goto exit;
      }

      if (!m_hasApi)
      {
        mReady.send("resolve", this);
        rtObjectRef e = new rtMapObject;
        e.set("name", "onReady");
        e.set("target", this);
        mEmit.send("onReady", e);
      }
      
      if ( strlen(cmd) > 0 )
      {
         rtLogDebug("pxWayland: launching client (%s)", cmd );
         rtLogInfo("remote client's id is %s", m_remoteObjectName.cString() );
         setenv("PX_WAYLAND_CLIENT_REMOTE_OBJECT_NAME", m_remoteObjectName.cString(), 1);
         launchClient();
         m_waitingForRemoteObject = true;
         startRemoteObjectDetection();
      }
   }
   
exit:
   if ( error )
   {
      const char *detail= WstCompositorGetLastErrorDetail( m_wctx );
      rtLogError("Compositor error: (%s)\n", detail );
   }
}

void pxWayland::draw() {
  if ( (m_fbo->width() != mw) ||
       (m_fbo->height() != mh) )
  {     
     context.updateFramebuffer( m_fbo, mw, mh );
     WstCompositorSetOutputSize( m_wctx, mw, mh );
  }
  
  pxMatrix4f m;
  context.pushState();
  pxContextFramebufferRef previousFrameBuffer= context.getCurrentFramebuffer();
  context.setFramebuffer( m_fbo );
  context.clear( mw, mh, m_fillColor );
  WstCompositorComposeEmbedded( m_wctx, 
                                mw,
                                mh,
                                mw,
                                mh,
                                m.data(),
                                context.getAlpha() );
  context.setFramebuffer( previousFrameBuffer );
  context.popState();
  
  static pxTextureRef nullMaskRef;
  context.drawImage(0, 0, mw, mh, m_fbo->getTexture(), nullMaskRef);
}

void pxWayland::onKeyDown(uint32_t keycode, uint32_t flags)
{
   int32_t linuxKeyCode;
   int32_t modifiers;

   modifiers= getModifiers(flags);
   linuxKeyCode= linuxFromPX( keycode );
   
   WstCompositorKeyEvent( m_wctx, linuxKeyCode, WstKeyboard_keyState_depressed, modifiers ); 
}

void pxWayland::onKeyUp(uint32_t keycode, uint32_t flags)
{
   int32_t linuxKeyCode;
   int32_t modifiers;
   
   modifiers= getModifiers(flags);
   linuxKeyCode= linuxFromPX( keycode );
   
   WstCompositorKeyEvent( m_wctx, linuxKeyCode, WstKeyboard_keyState_released, modifiers ); 
}

void pxWayland::onMouseEnter()
{
   WstCompositorPointerEnter( m_wctx );
}

void pxWayland::onMouseLeave()
{
   WstCompositorPointerLeave( m_wctx );
}


void pxWayland::onMouseMove( float x, float y )
{
   int objX, objY;
   int resW, resH;
   context.getSize( resW, resH );
   
   objX= (int)(x+0.5f);
   objY= (int)(y+0.5f);
   
   WstCompositorPointerMoveEvent( m_wctx, objX, objY );
}

void pxWayland::onMouseDown()
{
   // TODO: 0x110 is BTN_LEFT : need mouse event to include button codes
   // perhaps with mapping from native to PX.  Wayland expects Linux button codes
   WstCompositorPointerButtonEvent( m_wctx, 0x110, WstPointer_buttonState_depressed );
}

void pxWayland::onMouseUp()
{
   // TODO: 0x110 is BTN_LEFT : need mouse event to include button codes
   // perhaps with mapping from native to PX.  Wayland expects Linux button codes
   WstCompositorPointerButtonEvent( m_wctx, 0x110, WstPointer_buttonState_released );
}

void pxWayland::invalidate( WstCompositor *wctx, void *userData )
{
   (void)wctx;
   
   pxWayland *pxw= (pxWayland*)userData;
   
   pxw->mScene->invalidateRect(NULL);
   pxw->mScene->mDirty = true;
}

void pxWayland::hidePointer( WstCompositor *wctx, bool hide, void *userData )
{
   (void)wctx;

   pxWayland *pxw= (pxWayland*)userData;
   
   pxw->mScene->hidePointer( hide );
}

void pxWayland::clientStatus( WstCompositor *wctx, int status, int pid, int detail, void *userData )
{
   (void)wctx;
   
   pxWayland *pxw= (pxWayland*)userData;

   switch ( status )
   {
      case WstClient_started:
         pxw->handleClientStarted( pid );   
         break;
      case WstClient_stoppedNormal:
         pxw->handleClientStoppedNormal( pid, detail );   
         break;
      case WstClient_stoppedAbnormal:
         pxw->handleClientStoppedAbnormal( pid, detail );   
         break;
      case WstClient_connected:
         pxw->handleClientConnected( pid );   
         break;
      case WstClient_disconnected:
         pxw->handleClientDisconnected( pid );   
         break;
      default:
         rtLogError("unexpected wayland client status type");
         break;
   }
}

void pxWayland::handleClientStarted( int pid )
{
   m_clientPID= pid;
   
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientStarted");
   e.set("target", this);
   e.set("pid", pid );
   mEmit.send("onClientStarted", e);
}

void pxWayland::handleClientStoppedNormal( int pid, int exitCode )
{
   m_clientPID= -1;
   
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientStopped");
   e.set("target", this);
   e.set("pid", pid );
   e.set("crashed", false );
   e.set("exitCode", exitCode );
   mEmit.send("onClientStopped", e);
}

void pxWayland::handleClientStoppedAbnormal( int pid, int signo )
{
   m_clientPID= -1;
   
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientStopped");
   e.set("target", this);
   e.set("pid", pid );
   e.set("crashed", true );
   e.set("signo", signo );
   mEmit.send("onClientStopped", e);
}

void pxWayland::handleClientConnected( int pid )
{
   m_clientPID= pid;
 
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientConnected");
   e.set("target", this);
   e.set("pid", pid );
   mEmit.send("onClientConnected", e);
}

void pxWayland::handleClientDisconnected( int pid )
{
   m_clientPID= pid;
   
   rtObjectRef e = new rtMapObject;
   e.set("name", "onClientDisconnected");
   e.set("target", this);
   e.set("pid", pid );
   mEmit.send("onClientDisconnected", e);
}

uint32_t pxWayland::getModifiers( uint32_t flags )
{
   uint32_t modifiers= 0;
   
   if ( flags & PX_MOD_SHIFT )
      modifiers |= WstKeyboard_shift;
   if ( flags & PX_MOD_CONTROL )
      modifiers |= WstKeyboard_ctrl;
   if ( flags & PX_MOD_ALT )
      modifiers |= WstKeyboard_alt;
      
   return modifiers;
}

void pxWayland::launchClient()
{
   int rc;
   
   rc= pthread_create( &m_clientMonitorThreadId, NULL, clientMonitorThread, this );
   if ( rc )
   {
     const char* cmd = m_cmd.cString();
     rtLogError("Failed to start client monitor thread: auto launch of (%s) failed", cmd);      
   }
}

void pxWayland::launchAndMonitorClient()
{
   rtLogInfo( "pxWayland::launchAndMonitorClient enter for (%s)", m_cmd.cString() );

   m_clientMonitorStarted= true;

   if ( !WstCompositorLaunchClient( m_wctx, m_cmd.cString() ) )
   {
      rtLogError( "pxWayland::launchAndMonitorClient: WstCompositorLaunchClient failed for (%s)", m_cmd.cString() );
      const char *detail= WstCompositorGetLastErrorDetail( m_wctx );
      rtLogError("Compositor error: (%s)\n", detail );
   }

   m_clientMonitorStarted= false;

   rtLogInfo( "pxWayland::launchAndMonitorClient exit for (%s)", m_cmd.cString() );
}

void pxWayland::terminateClient()
{
   if ( m_clientMonitorStarted )
   {
      m_clientMonitorStarted= false;
      
      // Destroying compositor above should result in client 
      // process ending
      pthread_join( m_clientMonitorThreadId, NULL );      
   }

   if (m_waitingForRemoteObject)
   {
     m_waitingForRemoteObject = false;

     pthread_join( m_findRemoteThreadId, NULL );
   }
}

void* pxWayland::clientMonitorThread( void *data )
{
   pxWayland *pxw= (pxWayland*)data;
   
   pxw->launchAndMonitorClient();
   
   return NULL;
}

void pxWayland::startRemoteObjectDetection()
{
  int rc= pthread_create( &m_findRemoteThreadId, NULL, findRemoteThread, this );
  if ( rc )
  {
    rtLogError("Failed to start find remote object thread");
  }
}

void* pxWayland::findRemoteThread( void *data )
{
  pxWayland *pxw= (pxWayland*)data;

  rtError errorCode = pxw->startRemoteObjectLocator();

  if (errorCode == RT_OK)
  {
    pxw->connectToRemoteObject();
  }

  return NULL;
}

rtError pxWayland::startRemoteObjectLocator()
{
  rtError errorCode = m_locator.open();

  if (errorCode != RT_OK)
  {
    rtLogError("pxWayland failed to open rtRemoteObjectLocator: %d", errorCode);
    m_remoteObjectMutex.lock();
    m_waitingForRemoteObject = false;
    m_remoteObjectMutex.unlock();
    return errorCode;
  }

  errorCode = m_locator.start();
  if (errorCode != RT_OK)
  {
    rtLogError("pxWayland failed to start rtRemoteObjectLocator: %d", errorCode);
    m_remoteObjectMutex.lock();
    m_waitingForRemoteObject = false;
    m_remoteObjectMutex.unlock();
    return errorCode;
  }

  return errorCode;
}

rtError pxWayland::connectToRemoteObject()
{
  rtError errorCode = RT_FAIL;
  int findTime = 0;

  while (findTime < MAX_FIND_REMOTE_TIMEOUT_IN_MS)
  {
    findTime += FIND_REMOTE_ATTEMPT_TIMEOUT_IN_MS;
    rtLogInfo("Attempting to find remote object %s", m_remoteObjectName.cString());
    errorCode = m_locator.findObject(m_remoteObjectName.cString(), m_remoteObject, FIND_REMOTE_ATTEMPT_TIMEOUT_IN_MS);
    if (errorCode != RT_OK)
    {
      rtLogError("XREBrowserPlugin failed to find object: %s errorCode %d\n",
                 m_remoteObjectName.cString(), errorCode);
    }
    else
    {
      rtLogInfo("Remote object %s found.  search time %d ms \n", m_remoteObjectName.cString(), findTime);
      break;
    }
  }

  if (errorCode == RT_OK)
  {
    m_remoteObject.send("init");
    //rtString urlToSet = "http://www.google.com";
    //m_remoteObject.set("url", urlToSet);
    m_remoteObjectMutex.lock();
    m_API = m_remoteObject;
    m_remoteObjectMutex.unlock();
  }
  else
  {
    rtLogError("unable to connect to remote object");
  }

  m_remoteObjectMutex.lock();
  m_waitingForRemoteObject = false;
  m_remoteObjectMutex.unlock();

  return errorCode;
}

// These key codes are from linux/input.h which may not be available depending on what platform we are building for
#define KEY_RESERVED            0
#define KEY_ESC                 1
#define KEY_1                   2
#define KEY_2                   3
#define KEY_3                   4
#define KEY_4                   5
#define KEY_5                   6
#define KEY_6                   7
#define KEY_7                   8
#define KEY_8                   9
#define KEY_9                   10
#define KEY_0                   11
#define KEY_MINUS               12
#define KEY_EQUAL               13
#define KEY_BACKSPACE           14
#define KEY_TAB                 15
#define KEY_Q                   16
#define KEY_W                   17
#define KEY_E                   18
#define KEY_R                   19
#define KEY_T                   20
#define KEY_Y                   21
#define KEY_U                   22
#define KEY_I                   23
#define KEY_O                   24
#define KEY_P                   25
#define KEY_LEFTBRACE           26
#define KEY_RIGHTBRACE          27
#define KEY_ENTER               28
#define KEY_LEFTCTRL            29
#define KEY_A                   30
#define KEY_S                   31
#define KEY_D                   32
#define KEY_F                   33
#define KEY_G                   34
#define KEY_H                   35
#define KEY_J                   36
#define KEY_K                   37
#define KEY_L                   38
#define KEY_SEMICOLON           39
#define KEY_APOSTROPHE          40
#define KEY_GRAVE               41
#define KEY_LEFTSHIFT           42
#define KEY_BACKSLASH           43
#define KEY_Z                   44
#define KEY_X                   45
#define KEY_C                   46
#define KEY_V                   47
#define KEY_B                   48
#define KEY_N                   49
#define KEY_M                   50
#define KEY_COMMA               51
#define KEY_DOT                 52
#define KEY_SLASH               53
#define KEY_RIGHTSHIFT          54
#define KEY_KPASTERISK          55
#define KEY_LEFTALT             56
#define KEY_SPACE               57
#define KEY_CAPSLOCK            58
#define KEY_F1                  59
#define KEY_F2                  60
#define KEY_F3                  61
#define KEY_F4                  62
#define KEY_F5                  63
#define KEY_F6                  64
#define KEY_F7                  65
#define KEY_F8                  66
#define KEY_F9                  67
#define KEY_F10                 68
#define KEY_NUMLOCK             69
#define KEY_SCROLLLOCK          70
#define KEY_KP7                 71
#define KEY_KP8                 72
#define KEY_KP9                 73
#define KEY_KPMINUS             74
#define KEY_KP4                 75
#define KEY_KP5                 76
#define KEY_KP6                 77
#define KEY_KPPLUS              78
#define KEY_KP1                 79
#define KEY_KP2                 80
#define KEY_KP3                 81
#define KEY_KP0                 82
#define KEY_KPDOT               83
#define KEY_102ND               86
#define KEY_F11                 87
#define KEY_F12                 88
#define KEY_KPENTER             96
#define KEY_RIGHTCTRL           97
#define KEY_KPSLASH             98
#define KEY_RIGHTALT            100
#define KEY_HOME                102
#define KEY_UP                  103
#define KEY_PAGEUP              104
#define KEY_LEFT                105
#define KEY_RIGHT               106
#define KEY_END                 107
#define KEY_DOWN                108
#define KEY_PAGEDOWN            109
#define KEY_INSERT              110
#define KEY_DELETE              111
#define KEY_KPEQUAL             117
#define KEY_KPPLUSMINUS         118
#define KEY_PAUSE               119
#define KEY_KPCOMMA             121
#define KEY_LEFTMETA            125
#define KEY_RIGHTMETA           126
#define KEY_PRINT               210     /* AC Print */

uint32_t pxWayland::linuxFromPX( uint32_t keyCode )
{
   uint32_t  linuxKeyCode;
   
   switch( keyCode )
   {
      case PX_KEY_BACKSPACE:
         linuxKeyCode= KEY_BACKSPACE;
         break;
      case PX_KEY_TAB:
         linuxKeyCode= KEY_TAB;
         break;
      case PX_KEY_ENTER:
         linuxKeyCode= KEY_ENTER;
         break;
      case PX_KEY_SHIFT:
         linuxKeyCode= KEY_LEFTSHIFT;
         break;
      case PX_KEY_CTRL:
         linuxKeyCode= KEY_LEFTCTRL;
         break;
      case PX_KEY_ALT:
         linuxKeyCode= KEY_LEFTALT;
         break;
      case PX_KEY_PAUSE:
         linuxKeyCode= KEY_PAUSE;
         break;
      case PX_KEY_CAPSLOCK:
         linuxKeyCode= KEY_CAPSLOCK;
         break;
      case PX_KEY_ESCAPE:
         linuxKeyCode= KEY_ESC;
         break;
      case PX_KEY_SPACE:
         linuxKeyCode= KEY_SPACE;
         break;
      case PX_KEY_PAGEUP:
         linuxKeyCode= KEY_PAGEUP;
         break;
      case PX_KEY_PAGEDOWN:
         linuxKeyCode= KEY_SPACE;
         break;
      case PX_KEY_END:
         linuxKeyCode= KEY_END;
         break;
      case PX_KEY_HOME:
         linuxKeyCode= KEY_HOME;
         break;
      case PX_KEY_LEFT:
         linuxKeyCode= KEY_LEFT;
         break;
      case PX_KEY_UP:
         linuxKeyCode= KEY_UP;
         break;
      case PX_KEY_RIGHT:
         linuxKeyCode= KEY_RIGHT;
         break;
      case PX_KEY_DOWN:
         linuxKeyCode= KEY_DOWN;
         break;
      case PX_KEY_INSERT:
         linuxKeyCode= KEY_INSERT;
         break;
      case PX_KEY_DELETE:
         linuxKeyCode= KEY_DELETE;
         break;
      case PX_KEY_ZERO:
         linuxKeyCode= KEY_0;
         break;
      case PX_KEY_ONE:
         linuxKeyCode= KEY_1;
         break;
      case PX_KEY_TWO:
         linuxKeyCode= KEY_2;
         break;
      case PX_KEY_THREE:
         linuxKeyCode= KEY_3;
         break;
      case PX_KEY_FOUR:
         linuxKeyCode= KEY_4;
         break;
      case PX_KEY_FIVE:
         linuxKeyCode= KEY_5;
         break;
      case PX_KEY_SIX:
         linuxKeyCode= KEY_6;
         break;
      case PX_KEY_SEVEN:
         linuxKeyCode= KEY_7;
         break;
      case PX_KEY_EIGHT:
         linuxKeyCode= KEY_8;
         break;
      case PX_KEY_NINE:
         linuxKeyCode= KEY_9;
         break;
      case PX_KEY_A:
         linuxKeyCode= KEY_A;
         break;
      case PX_KEY_B:
         linuxKeyCode= KEY_B;
         break;
      case PX_KEY_C:
         linuxKeyCode= KEY_C;
         break;
      case PX_KEY_D:
         linuxKeyCode= KEY_D;
         break;
      case PX_KEY_E:
         linuxKeyCode= KEY_E;
         break;
      case PX_KEY_F:
         linuxKeyCode= KEY_F;
         break;
      case PX_KEY_G:
         linuxKeyCode= KEY_G;
         break;
      case PX_KEY_H:
         linuxKeyCode= KEY_H;
         break;
      case PX_KEY_I:
         linuxKeyCode= KEY_I;
         break;
      case PX_KEY_J:
         linuxKeyCode= KEY_J;
         break;
      case PX_KEY_K:
         linuxKeyCode= KEY_K;
         break;
      case PX_KEY_L:
         linuxKeyCode= KEY_L;
         break;
      case PX_KEY_M:
         linuxKeyCode= KEY_M;
         break;
      case PX_KEY_N:
         linuxKeyCode= KEY_N;
         break;
      case PX_KEY_O:
         linuxKeyCode= KEY_O;
         break;
      case PX_KEY_P:
         linuxKeyCode= KEY_P;
         break;
      case PX_KEY_Q:
         linuxKeyCode= KEY_Q;
         break;
      case PX_KEY_R:
         linuxKeyCode= KEY_R;
         break;
      case PX_KEY_S:
         linuxKeyCode= KEY_S;
         break;
      case PX_KEY_T:
         linuxKeyCode= KEY_T;
         break;
      case PX_KEY_U:
         linuxKeyCode= KEY_U;
         break;
      case PX_KEY_V:
         linuxKeyCode= KEY_V;
         break;
      case PX_KEY_W:
         linuxKeyCode= KEY_W;
         break;
      case PX_KEY_X:
         linuxKeyCode= KEY_X;
         break;
      case PX_KEY_Y:
         linuxKeyCode= KEY_Y;
         break;
      case PX_KEY_Z:
         linuxKeyCode= KEY_Z;
         break;
      case PX_KEY_NUMPAD0:
         linuxKeyCode= KEY_KP0;
         break;
      case PX_KEY_NUMPAD1:
         linuxKeyCode= KEY_KP1;
         break;
      case PX_KEY_NUMPAD2:
         linuxKeyCode= KEY_KP2;
         break;
      case PX_KEY_NUMPAD3:
         linuxKeyCode= KEY_KP3;
         break;
      case PX_KEY_NUMPAD4:
         linuxKeyCode= KEY_KP4;
         break;
      case PX_KEY_NUMPAD5:
         linuxKeyCode= KEY_KP5;
         break;
      case PX_KEY_NUMPAD6:
         linuxKeyCode= KEY_KP6;
         break;
      case PX_KEY_NUMPAD7:
         linuxKeyCode= KEY_KP7;
         break;
      case PX_KEY_NUMPAD8:
         linuxKeyCode= KEY_KP8;
         break;
      case PX_KEY_NUMPAD9:
         linuxKeyCode= KEY_KP9;
         break;
      case PX_KEY_MULTIPLY:
         linuxKeyCode= KEY_KPASTERISK;
         break;
      case PX_KEY_ADD:
         linuxKeyCode= KEY_KPPLUS;
         break;
      case PX_KEY_SUBTRACT:
         linuxKeyCode= KEY_MINUS;
         break;
      case PX_KEY_DECIMAL:
         linuxKeyCode= KEY_KPDOT;
         break;
      case PX_KEY_DIVIDE:
         linuxKeyCode= KEY_KPSLASH;
         break;
      case PX_KEY_F1:
         linuxKeyCode= KEY_F1;
         break;
      case PX_KEY_F2:
         linuxKeyCode= KEY_F2;
         break;
      case PX_KEY_F3:
         linuxKeyCode= KEY_F3;
         break;
      case PX_KEY_F4:
         linuxKeyCode= KEY_F4;
         break;
      case PX_KEY_F5:
         linuxKeyCode= KEY_F5;
         break;
      case PX_KEY_F6:
         linuxKeyCode= KEY_F6;
         break;
      case PX_KEY_F7:
         linuxKeyCode= KEY_F7;
         break;
      case PX_KEY_F8:
         linuxKeyCode= KEY_F8;
         break;
      case PX_KEY_F9:
         linuxKeyCode= KEY_F9;
         break;
      case PX_KEY_F10:
         linuxKeyCode= KEY_F10;
         break;
      case PX_KEY_F11:
         linuxKeyCode= KEY_F11;
         break;
      case PX_KEY_F12:
         linuxKeyCode= KEY_F12;
         break;
      case PX_KEY_NUMLOCK:
         linuxKeyCode= KEY_NUMLOCK;
         break;
      case PX_KEY_SCROLLLOCK:
         linuxKeyCode= KEY_SCROLLLOCK;
         break;
      case PX_KEY_SEMICOLON:
         linuxKeyCode= KEY_SEMICOLON;
         break;
      case PX_KEY_EQUALS:
         linuxKeyCode= KEY_EQUAL;
         break;
      case PX_KEY_COMMA:
         linuxKeyCode= KEY_COMMA;
         break;
      case PX_KEY_PERIOD:
         linuxKeyCode= KEY_DOT;
         break;
      case PX_KEY_FORWARDSLASH:
         linuxKeyCode= KEY_SLASH;
         break;
      case PX_KEY_GRAVEACCENT:
         linuxKeyCode= KEY_GRAVE;
         break;
      case PX_KEY_OPENBRACKET:
         linuxKeyCode= KEY_LEFTBRACE;
         break;
      case PX_KEY_BACKSLASH:
         linuxKeyCode= KEY_BACKSLASH;
         break;
      case PX_KEY_CLOSEBRACKET:
         linuxKeyCode= KEY_RIGHTBRACE;
         break;
      case PX_KEY_SINGLEQUOTE:
         linuxKeyCode= KEY_APOSTROPHE;
         break;
      case PX_KEY_PRINTSCREEN:
         linuxKeyCode= KEY_PRINT;
         break;
      case PX_KEY_DASH:
         linuxKeyCode= KEY_MINUS;
         break;
      default:
         linuxKeyCode= -1;
         break;
   }
   
   return  linuxKeyCode;
}

rtDefineObject(pxWayland,pxObject);
rtDefineProperty(pxWayland,displayName);
rtDefineProperty(pxWayland,cmd);
rtDefineProperty(pxWayland, clientPID);
rtDefineProperty(pxWayland, fillColor);
rtDefineProperty(pxWayland, api);


