// pxCore CopyRight 2007-2015 John Robinson
// pxWayland.cpp

#include "rtString.h"
#include "rtRefT.h"
#include "pxCore.h"
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

#define UNUSED_PARAM(x) ((x)=(x))

pxWayland::pxWayland()
  :
    mClientMonitorThreadId(0), 
    mFindRemoteThreadId(0),
    mContainer(NULL),
    mReadyEmitted(false),
    mClientMonitorStarted(false),
    mWaitingForRemoteObject(false),
    mX(0),
    mY(0),
    mWidth(0),
    mHeight(0),
    mEvents(0),
    mClientPID(-1),
    mWCtx(0),
    mHasApi(false),
    mAPI(),
#ifdef ENABLE_PX_WAYLAND_RPC
    mRemoteObject(),
#endif //ENABLE_PX_WAYLAND_RPC
    mRemoteObjectMutex()
{
  mFillColor[0]= 0.0; 
  mFillColor[1]= 0.0; 
  mFillColor[2]= 0.0; 
  mFillColor[3]= 0.0; 
}

pxWayland::~pxWayland()
{ 
  if ( mWCtx )
  {
     WstCompositorDestroy(mWCtx);
     terminateClient();
  }
}

rtError pxWayland::displayName(rtString& s) const { s = mDisplayName; return RT_OK; }
rtError pxWayland::setDisplayName(const char* s) 
{
  mDisplayName = s;
  return RT_OK;
}

void pxWayland::onInit()
{
  createDisplay(mDisplayName);
}

void pxWayland::createDisplay(rtString displayName)
{
   bool error= false;
   const char* name= displayName.cString();
   const char* cmd= mCmd.cString();

   rtLogInfo("pxWayland::createDisplay: %s\n", (name ? name : "name not provided"));
   
   mFBO= context.createFramebuffer( 0, 0 );
   
   mWCtx= WstCompositorCreate();
   if ( mWCtx )
   {
      if ( !WstCompositorSetIsEmbedded( mWCtx, true ) )
      {
         error= true;
         goto exit;
      }
      
      if ( !WstCompositorSetOutputSize( mWCtx, mWidth, mHeight ) )
      {
         error= true;
         goto exit;
      }
      
      if ( !WstCompositorSetInvalidateCallback( mWCtx, invalidate, this ) )
      {
         error= true;
         goto exit;
      }
      
      if ( !WstCompositorSetHidePointerCallback( mWCtx, hidePointer, this ) )
      {
         error= true;
         goto exit;
      }
      
      if ( !WstCompositorSetClientStatusCallback( mWCtx, clientStatus, this ) )
      {
         error= true;
         goto exit;
      }
      
      // If a display name was provided, use it.  Otherwise the
      // compositor will generate a unique display name to use.
      int len= (name ? strlen(name) : 0);
      if ( len > 0 )
      {
         if ( !WstCompositorSetDisplayName( mWCtx, name ) )
         {
            error= true;
            goto exit;
         }
      }
      if (mDisplayName.isEmpty())
          mDisplayName = WstCompositorGetDisplayName(mWCtx);
      if(mRemoteObjectName.isEmpty())
      {
          mRemoteObjectName = "wl-plgn-";
          mRemoteObjectName.append(mDisplayName.cString());
      }
      
      if ( !WstCompositorStart( mWCtx ) )
      {
         error= true;
         goto exit;
      }

      if (!mHasApi && mEvents )
      {
        mReadyEmitted= true;
        mEvents->isReady( true );
      }
      
      if ( strlen(cmd) > 0 )
      {
         setenv("WAYLAND_DISPLAY", mDisplayName.cString(), 1);
         setenv("PX_WAYLAND_CLIENT_REMOTE_OBJECT_NAME", mRemoteObjectName.cString(), 1);

         rtLogDebug("pxWayland: launching client (%s)", cmd );
         rtLogInfo("remote client's id is %s", mRemoteObjectName.cString() );
         launchClient();
         mWaitingForRemoteObject = true;
         startRemoteObjectDetection();
      }
   }
   
exit:
   if ( error )
   {
      const char *detail= WstCompositorGetLastErrorDetail( mWCtx );
      rtLogError("Compositor error: (%s)\n", detail );
   }
}

void pxWayland::onSize(int32_t w, int32_t h)
{
  mWidth  = w;
  mHeight = h;
}

bool pxWayland::onMouseDown(int32_t x, int32_t y, uint32_t flags)
{
   UNUSED_PARAM(x);
   UNUSED_PARAM(y);
   UNUSED_PARAM(flags);

   // TODO: 0x110 is BTN_LEFT : need mouse event to include button codes
   // perhaps with mapping from native to PX.  Wayland expects Linux button codes
   WstCompositorPointerButtonEvent( mWCtx, 0x110, WstPointer_buttonState_depressed );
   return false;
}

bool pxWayland::onMouseUp(int32_t x, int32_t y, uint32_t flags)
{
   UNUSED_PARAM(x);
   UNUSED_PARAM(y);
   UNUSED_PARAM(flags);

   // TODO: 0x110 is BTN_LEFT : need mouse event to include button codes
   // perhaps with mapping from native to PX.  Wayland expects Linux button codes
   WstCompositorPointerButtonEvent( mWCtx, 0x110, WstPointer_buttonState_released );
   return false;
}

bool pxWayland::onMouseEnter()
{
   WstCompositorPointerEnter( mWCtx );
   return false;
}

bool pxWayland::onMouseLeave()
{
   WstCompositorPointerLeave( mWCtx );
   return false;
}

bool pxWayland::onMouseMove(int32_t x, int32_t y)
{
   int objX, objY;
   int resW, resH;
   context.getSize( resW, resH );
   
   objX= (int)(x+0.5f);
   objY= (int)(y+0.5f);
   
   WstCompositorPointerMoveEvent( mWCtx, objX, objY );
   return false;
}

bool pxWayland::onFocus()
{
   // Nothing to do
  return false;
}

bool pxWayland::onBlur()
{
   // Nothing to do
  return false;
}

bool pxWayland::onKeyDown(uint32_t keycode, uint32_t flags)
{
   int32_t linuxKeyCode;
   int32_t modifiers;

   modifiers= getModifiers(flags);
   linuxKeyCode= linuxFromPX( keycode );
   
   WstCompositorKeyEvent( mWCtx, linuxKeyCode, WstKeyboard_keyState_depressed, modifiers ); 
   return false;
}

bool pxWayland::onKeyUp(uint32_t keycode, uint32_t flags)
{
   int32_t linuxKeyCode;
   int32_t modifiers;
   
   modifiers= getModifiers(flags);
   linuxKeyCode= linuxFromPX( keycode );
   
   WstCompositorKeyEvent( mWCtx, linuxKeyCode, WstKeyboard_keyState_released, modifiers ); 
   return false;
}

bool pxWayland::onChar(uint32_t codepoint)
{
   UNUSED_PARAM(codepoint);

   // Nothing to do
   return false;
}

void pxWayland::onUpdate(double t)
{
   UNUSED_PARAM(t);

  if(!mReadyEmitted && mEvents && mWCtx && !mWaitingForRemoteObject )
  {
    mReadyEmitted= true;
    mEvents->isReady(true);
  }
}

void pxWayland::onDraw()
{
  static pxTextureRef nullMaskRef;
  
  if ( (mFBO->width() != mWidth) ||
       (mFBO->height() != mHeight) )
  {     
     context.updateFramebuffer( mFBO, mWidth, mHeight );
     WstCompositorSetOutputSize( mWCtx, mWidth, mHeight );
  }
  
  int hints= 0;
  if ( !isRotated() ) hints |= WstHints_noRotation;
  
  bool needHolePunch;
  std::vector<WstRect> rects;
  pxMatrix4f m= context.getMatrix();
  context.pushState();
  pxContextFramebufferRef previousFrameBuffer= context.getCurrentFramebuffer();
  context.setFramebuffer( mFBO );
  context.clear( mWidth, mHeight, mFillColor );
  WstCompositorComposeEmbedded( mWCtx, 
                                mX,
                                mY,
                                mWidth,
                                mHeight,
                                m.data(),
                                context.getAlpha(),
                                hints,
                                &needHolePunch,
                                rects );
  context.setFramebuffer( previousFrameBuffer );
  context.popState();
  
  if ( needHolePunch )
  {
     if ( mFillColor[3] != 0.0 )
     {
        context.drawImage(0, 0, mWidth, mHeight, mFBO->getTexture(), nullMaskRef);
     }
     GLfloat priorColor[4];
     GLint priorBox[4];
     GLint viewport[4];
     bool wasEnabled= glIsEnabled(GL_SCISSOR_TEST);
     glGetIntegerv( GL_SCISSOR_BOX, priorBox );
     glGetFloatv( GL_COLOR_CLEAR_VALUE, priorColor );
     glGetIntegerv( GL_VIEWPORT, viewport );
     
     glEnable( GL_SCISSOR_TEST );
     glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
     for( unsigned int i= 0; i < rects.size(); ++i )
     {
        WstRect r= rects[i];
        if ( r.width && r.height )
        {
           glScissor( r.x, viewport[3]-(r.y+r.height), r.width, r.height );
           glClear( GL_COLOR_BUFFER_BIT );
        }
     }
     
     if ( wasEnabled )
     {
        glScissor( priorBox[0], priorBox[1], priorBox[2], priorBox[3] );
     }
     else
     {
        glDisable( GL_SCISSOR_TEST );
     }
  }
  context.drawImage(0, 0, mWidth, mHeight, mFBO->getTexture(), nullMaskRef);
}

void pxWayland::handleInvalidate()
{
   if ( mEvents )
   {
      mEvents->invalidate(NULL);
   }
}

void pxWayland::handleHidePointer( bool hide )
{
   if ( mEvents )
   {
      mEvents->hidePointer( hide );
   }
}

void pxWayland::handleClientStatus( int status, int pid, int detail )
{
   if ( mEvents )
   {
      switch ( status )
      {
         case WstClient_started:
            mEvents->clientStarted( pid );   
            break;
         case WstClient_stoppedNormal:
            mEvents->clientStoppedNormal( pid, detail );   
            break;
         case WstClient_stoppedAbnormal:
            mEvents->clientStoppedAbnormal( pid, detail );   
            break;
         case WstClient_connected:
            mEvents->clientConnected( pid );   
            break;
         case WstClient_disconnected:
            mEvents->clientDisconnected( pid );   
            break;
         default:
            rtLogError("unexpected wayland client status type");
            break;
      }
   }
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

bool pxWayland::isRotated()
{
   float *f= context.getMatrix().data();
   const float e= 1.0e-2;
      
   if ( (fabsf(f[1]) > e) ||
        (fabsf(f[2]) > e) ||
        (fabsf(f[4]) > e) ||
        (fabsf(f[6]) > e) ||
        (fabsf(f[8]) > e) ||
        (fabsf(f[9]) > e) )
   {
      return true;
   }
   
   return false;
}

void pxWayland::launchClient()
{
   int rc;
   
   rc= pthread_create( &mClientMonitorThreadId, NULL, clientMonitorThread, this );
   if ( rc )
   {
     const char* cmd = mCmd.cString();
     rtLogError("Failed to start client monitor thread: auto launch of (%s) failed", cmd);      
   }
}

void pxWayland::launchAndMonitorClient()
{
   rtLogInfo( "pxWayland::launchAndMonitorClient enter for (%s)", mCmd.cString() );

   mClientMonitorStarted= true;

   if ( !WstCompositorLaunchClient( mWCtx, mCmd.cString() ) )
   {
      rtLogError( "pxWayland::launchAndMonitorClient: WstCompositorLaunchClient failed for (%s)", mCmd.cString() );
      const char *detail= WstCompositorGetLastErrorDetail( mWCtx );
      rtLogError("Compositor error: (%s)\n", detail );
   }

   mClientMonitorStarted= false;

   rtLogInfo( "pxWayland::launchAndMonitorClient exit for (%s)", mCmd.cString() );
}

void pxWayland::terminateClient()
{
   if ( mClientMonitorStarted )
   {
      mClientMonitorStarted= false;
      
      // Destroying compositor above should result in client 
      // process ending
      pthread_join( mClientMonitorThreadId, NULL );      
   }

   if (mWaitingForRemoteObject)
   {
      mWaitingForRemoteObject = false;

      pthread_join( mFindRemoteThreadId, NULL );
   }
}

void* pxWayland::clientMonitorThread( void *data )
{
   pxWayland *pxw= (pxWayland*)data;
   
   pxw->launchAndMonitorClient();
   
   return NULL;
}

void pxWayland::invalidate( WstCompositor *wctx, void *userData )
{
   (void)wctx;
   
   pxWayland *pxw= (pxWayland*)userData;
   pxw->handleInvalidate();
}

void pxWayland::hidePointer( WstCompositor *wctx, bool hide, void *userData )
{
   (void)wctx;

   pxWayland *pxw= (pxWayland*)userData;
   pxw->handleHidePointer( hide );
}

void pxWayland::clientStatus( WstCompositor *wctx, int status, int pid, int detail, void *userData )
{
   (void)wctx;
   
   pxWayland *pxw= (pxWayland*)userData;

   pxw->handleClientStatus( status, pid, detail );
}

void pxWayland::startRemoteObjectDetection()
{
  int rc= pthread_create( &mFindRemoteThreadId, NULL, findRemoteThread, this );
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
#ifdef ENABLE_PX_WAYLAND_RPC
  rtError errorCode = rtRemoteInit();
  if (errorCode != RT_OK)
  {
    rtLogError("pxWayland failed to initialize rtRemoteInit: %d", errorCode);
    mRemoteObjectMutex.lock();
    mWaitingForRemoteObject = false;
    mRemoteObjectMutex.unlock();
    return errorCode;
  }

  return errorCode;
#else
  return RT_FAIL;
#endif //ENABLE_PX_WAYLAND_RPC
}

rtError pxWayland::connectToRemoteObject()
{
  rtError errorCode = RT_FAIL;
#ifdef ENABLE_PX_WAYLAND_RPC
  int findTime = 0;

  while (findTime < MAX_FIND_REMOTE_TIMEOUT_IN_MS)
  {
    findTime += FIND_REMOTE_ATTEMPT_TIMEOUT_IN_MS;
    rtLogInfo("Attempting to find remote object %s", mRemoteObjectName.cString());
    errorCode = rtRemoteLocateObject(mRemoteObjectName.cString(), mRemoteObject);
    if (errorCode != RT_OK)
    {
      rtLogError("XREBrowserPlugin failed to find object: %s errorCode %d\n",
                 mRemoteObjectName.cString(), errorCode);
    }
    else
    {
      rtLogInfo("Remote object %s found.  search time %d ms \n", mRemoteObjectName.cString(), findTime);
      break;
    }
  }

  if (errorCode == RT_OK)
  {
    mRemoteObject.send("init");
    mRemoteObjectMutex.lock();
    mAPI = mRemoteObject;
    mRemoteObjectMutex.unlock();

    if(mEvents)
        mEvents->isRemoteReady(true);
  }
  else
  {
    rtLogError("unable to connect to remote object");
  }

  mRemoteObjectMutex.lock();
  mWaitingForRemoteObject = false;
  mRemoteObjectMutex.unlock();
#endif //ENABLE_PX_WAYLAND_RPC
  return errorCode;
}

rtError pxWayland::setProperty(rtString &prop, rtString &val) const
{
  rtError errorCode = RT_FAIL;
#ifdef ENABLE_PX_WAYLAND_RPC
  if(mRemoteObject)
      errorCode = mRemoteObject.set(prop, val);
#endif //ENABLE_PX_WAYLAND_RPC
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

pxWaylandContainer::pxWaylandContainer(pxScene2d* scene)
   : pxViewContainer(scene), mWayland(NULL)
{
  addListener("onClientStarted", get<rtFunctionRef>("onClientStarted"));
  addListener("onClientStopped", get<rtFunctionRef>("onClientStopped"));
  addListener("onClientConnected", get<rtFunctionRef>("onClientConnected"));
  addListener("onClientDisconnected", get<rtFunctionRef>("onClientDisconnected"));
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

void pxWaylandContainer::onInit()
{
  if ( mWayland )
  {
    mWayland->setPos( mx, my );
    mWayland->onInit();
  }
}

rtDefineObject(pxWaylandContainer,pxViewContainer);
rtDefineProperty(pxWaylandContainer,displayName);
rtDefineProperty(pxWaylandContainer,cmd);
rtDefineProperty(pxWaylandContainer,clientPID);
rtDefineProperty(pxWaylandContainer,fillColor);

