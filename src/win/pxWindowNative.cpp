/*

pxCore Copyright 2005-2021 John Robinson

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

// pxWindowNative.cpp

#define PX_NATIVE

#include "rtLog.h"
#include "pxOffscreenNative.h"
#include "pxWindowNative.h"

#include "../pxKeycodes.h"
#include "../pxWindow.h"
#include "../pxWindowUtil.h"


#ifndef WINCE
#include <tchar.h>
#define _ATL_NO_HOSTING
//#include <atlconv.h>
#endif

#include "windowsx.h"

#if 1
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
#include <GLES2/gl2.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#ifdef WIN32
#include <GL/wglew.h>
#endif // WIN32
#ifdef PX_PLATFORM_GLUT
#include <GL/glut.h>
#endif
#include <GL/gl.h>
#endif //PX_PLATFORM_WAYLAND_EGL
#endif
#endif

#pragma warning(disable : 4311)
#pragma warning(disable : 4312)

#define WM_DEFERREDCREATE WM_USER + 1000
// With the addition of getting native events
// maybe this should be deprecated?
#define WM_SYNCHRONIZEDMESSAGE WM_USER + 1001

#ifdef WINCE
#define MOBILE
#include "aygshell.h"
#endif

using namespace std;

void setWindowPtr(HWND w, void* p) {
#ifndef WINCE
  SetWindowLongPtr(w, GWLP_USERDATA, (LONG)p);
#else
  SetWindowLong(w, GWL_USERDATA, (LONG)p);
#endif
}

void* getWindowPtr(HWND w) {
#ifndef WINCE
  return (void*)GetWindowLongPtr(w, GWLP_USERDATA);
#else
  return (void*)GetWindowLong(w, GWL_USERDATA);
#endif
}

// pxWindow

pxError pxWindow::init(int left, int top, int width, int height) {
#ifndef MOBILE
  return initNative(NULL, left, top, width, height, WS_OVERLAPPEDWINDOW, 0);
#else
  return initNative(NULL, left, top, width, height, WS_VISIBLE, 0);
#endif
}

#include <string>

pxError pxWindowNative::initNative(HWND parent, int left, int top, int width,
                                   int height, DWORD style, DWORD styleEx) {
  HINSTANCE hInstance = ::GetModuleHandle(NULL);

  std::string className = "pxWindow";
  WNDCLASS wc;
  if (!::GetClassInfo(hInstance, className.c_str(), &wc)) {
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC)windowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = className.c_str();

    RegisterClass(&wc);
  }

#ifndef MOBILE
  mWindow =
      ::CreateWindowEx(styleEx, "pxWindow", "", style, left, top, width, height,
                       parent, NULL, hInstance, (pxWindowNative*)this);
#else
  mWindow = CreateWindow(className.c_str(), L"", style, CW_USEDEFAULT,
                         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
                         NULL, hInstance, (pxWindowNative*)this);

  if (mWindow) {
    SHDoneButton(mWindow, SHDB_SHOW);
    SHFullScreen(mWindow, SHFS_HIDESIPBUTTON);
  }
#endif

#if 1
  HDC hdc = ::GetDC(mWindow);
  HGLRC hrc;

  static PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR),
                                      1,
                                      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                                          PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE,
                                      PFD_TYPE_RGBA,
                                      24,
                                      0,
                                      0,
                                      0,
                                      0,
                                      0,
                                      0,
                                      8,
                                      0,
                                      0,
                                      0,
                                      0,
                                      0,
                                      0,
                                      24,
                                      8,
                                      0,
                                      PFD_MAIN_PLANE,
                                      0,
                                      0,
                                      0,
                                      0};

  int pixelFormat = ChoosePixelFormat(hdc, &pfd);
  if (::SetPixelFormat(hdc, pixelFormat, &pfd)) {
    hrc = wglCreateContext(hdc);
    if (::wglMakeCurrent(hdc, hrc)) {
      glewExperimental = GL_TRUE;
      if (glewInit() != GLEW_OK) throw std::runtime_error("glewInit failed");

      char *GL_version = (char *)glGetString(GL_VERSION);
      char *GL_vendor = (char *)glGetString(GL_VENDOR);
      char *GL_renderer = (char *)glGetString(GL_RENDERER);

      rtLogInfo("GL_version = %s", GL_version);
      rtLogInfo("GL_vendor = %s", GL_vendor);
      rtLogInfo("GL_renderer = %s", GL_renderer);
    }
  }
#endif

  return mWindow ? PX_OK : PX_FAIL;
}

void pxWindowNative::size(int& width, int& height) {
  RECT r;
  GetClientRect(mWindow, &r);
  width = r.right - r.left;
  height = r.bottom - r.top;
}

pxError pxWindow::term() {
  //    ::DestroyWindow(mWindow);
  return PX_OK;
}

#if 1
void pxWindow::invalidateRect(pxRect* r) {
  RECT wr;
  RECT* pwr = NULL;
  if (r) {
    pwr = &wr;
    SetRect(pwr, r->left(), r->top(), r->right(), r->bottom());
  }
  InvalidateRect(getHWND(), pwr, FALSE);
}
#endif

bool pxWindow::visibility() {
  return IsWindowVisible(getHWND()) ? true : false;
}

void pxWindow::setVisibility(bool visible) {
  ShowWindow(getHWND(), visible ? SW_SHOW : SW_HIDE);
}

pxError pxWindow::setAnimationFPS(uint32_t fps) {
#if 0
    if (mTimerId)
    {
        KillTimer(mWindow, mTimerId);
        mTimerId = NULL;
    }

    if (fps > 0)
    {
        mTimerId = SetTimer(mWindow, 1, 1000/fps, NULL);
    }
    return PX_OK;
#else
  pxWindowNative::setAnimationFPS(fps);
  return PX_OK;
#endif
}

pxError pxWindowNative::setAnimationFPS(long fps) {
  if (mTimerId) {
    KillTimer(getHWND(), mTimerId);
    mTimerId = NULL;
  }

  if (fps > 0) {
    mTimerId = SetTimer(getHWND(), 1, 1000 / fps, NULL);
  }
  return PX_OK;
}

void pxWindow::setTitle(const char* title) {
#if 0
	USES_CONVERSION;
    ::SetWindowText(mWindow, A2T(title));
#else
#ifndef WINCE
  ::SetWindowTextA(getHWND(), title);
#endif
#endif
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& s) {
  s = ::GetDC(getHWND());
  return s ? PX_OK : PX_FAIL;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& s) {
  ::ReleaseDC(getHWND(), s);
  return PX_OK;
}

// pxWindowNative

pxWindowNative::~pxWindowNative() { setWindowPtr(mWindow, NULL); }

void pxWindowNative::sendSynchronizedMessage(char* messageName, void* p1) {
  synchronizedMessage m;
  m.messageName = messageName;
  m.p1 = p1;
  ::SendMessage(mWindow, WM_SYNCHRONIZEDMESSAGE, 0, (LPARAM)&m);
}

LRESULT __stdcall pxWindowNative::windowProc(HWND hWnd, UINT msg, WPARAM wParam,
                                             LPARAM lParam) {
  int mouseButtonShift = 0;

  pxWindowNative* w;

  static HDC hDC;
  static HGLRC hRC;

#ifndef WINCE
  if (msg == WM_NCCREATE)
#else
  if (msg == WM_CREATE)
#endif
  {
    CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
    // ::SetProp(hWnd, _T("wnWindow"), (HANDLE)cs->lpCreateParams);
    setWindowPtr(hWnd, cs->lpCreateParams);
    w = (pxWindowNative*)cs->lpCreateParams;
    w->mWindow = hWnd;
  } else
    w = (pxWindowNative*)getWindowPtr(hWnd);

  bool consumed = false;

  if (w) {
    pxEventNativeDesc e;

    e.wnd = hWnd;
    e.msg = msg;
    e.wParam = wParam;
    e.lParam = lParam;

    w->onEvent(e, consumed);

    if (consumed) {
      // ick
      if (e.msg == WM_SETCURSOR)
        return 1;
      else
        return 0;
    }

    // re resolve the window ptr since we have destroyed it

    w = (pxWindowNative*)getWindowPtr(hWnd);
    if (w) {
      switch (msg) {
// Special case code to handle the "fake" close button in the form
// of an OK button on win mobile
#ifdef MOBILE
        case WM_COMMAND: {
          unsigned int wmId = LOWORD(wParam);
          unsigned int wmEvent = HIWORD(wParam);
          // Parse the menu selections:
          switch (wmId) {
            case IDOK:
              SendMessage(w->mWindow, WM_CLOSE, 0, 0);
              break;
            default:
              return DefWindowProc(w->mWindow, msg, wParam, lParam);
          }
        } break;
#endif
#if 1
        case WM_CREATE:
          // We have to retry setting the animation timer if it happened to
          // early after creation
          w->onCreate();
          PostMessage(hWnd, WM_DEFERREDCREATE, 0, 0);
          break;
        case WM_DEFERREDCREATE:
          // w->onCreate();
          if (w->mAnimationFPS) w->setAnimationFPS(w->mAnimationFPS);
          break;

        case WM_CLOSE:
          w->onCloseRequest();
          return 0;
          break;

        case WM_DESTROY:
          w->onClose();
          // SetProp(hWnd, _T("wnWindow"), NULL);

          wglMakeCurrent(hDC, nullptr);
          wglDeleteContext(hRC);
#ifdef WINCE
          setWindowPtr(hWnd, NULL);
#endif
          break;

#ifndef WINCE
        case WM_NCDESTROY:
          setWindowPtr(hWnd, NULL);
          break;
#endif

        case WM_RBUTTONDOWN:
          mouseButtonShift++;
        case WM_MBUTTONDOWN:
          mouseButtonShift++;
        case WM_LBUTTONDOWN: {
#if 1
          SetCapture(w->mWindow);
          unsigned long flags = 1 << mouseButtonShift;
          if (GetKeyState(VK_CONTROL) < 0) flags |= PX_MOD_CONTROL;
          if (GetKeyState(VK_SHIFT) < 0) flags |= PX_MOD_SHIFT;
          if (GetKeyState(VK_MENU) < 0) flags |= PX_MOD_ALT;

          w->onMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), flags);

#endif
        } break;
        case WM_RBUTTONUP:
          mouseButtonShift++;
        case WM_MBUTTONUP:
          mouseButtonShift++;
        case WM_LBUTTONUP: {
#if 1
          ReleaseCapture();
          unsigned long flags = 1 << mouseButtonShift;
          if (GetKeyState(VK_CONTROL) < 0) flags |= PX_MOD_CONTROL;
          if (GetKeyState(VK_SHIFT) < 0) flags |= PX_MOD_SHIFT;
          if (GetKeyState(VK_MENU) < 0) flags |= PX_MOD_ALT;

          w->onMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), flags);
#endif
        } break;

        case WM_MOUSEWHEEL: {
          int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
          int direction = (int(wParam) > 0) ? 1 : -1;

          // w->onScrollWheel((float)(GET_X_LPARAM(lParam) * direction),
          // (float)(GET_Y_LPARAM(lParam) * direction));
          w->onScrollWheel((float)0, (float)zDelta > 0 ? 16 : -16);
          printf("zDelta: %d direction: %d x: %d y: %d\n", zDelta, direction,
                 GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        } break;

        case WM_MOUSEMOVE:
#if 1
#ifndef WINCE
          if (!w->mTrackMouse) {
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;

            if (TrackMouseEvent(&tme)) {
              w->mTrackMouse = true;
            }
          }
#endif
          w->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
#endif
          break;

#ifndef WINCE
        case WM_MOUSELEAVE:
          w->mTrackMouse = false;
          w->onMouseLeave();
          break;
#endif

        case WM_TIMER:
          // Should filter this to a single id
          w->onAnimationTimer();
          break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
          unsigned long flags = 0;

          if (GetKeyState(VK_SHIFT) & 0x8000) {
            flags |= PX_MOD_SHIFT;
          }
          if (GetKeyState(VK_CONTROL) & 0x8000) {
            flags |= PX_MOD_CONTROL;
          }
          if (GetKeyState(VK_MENU) & 0x8000) {
            flags |= PX_MOD_ALT;
          }

          uint32_t keyCode = keycodeFromNative((int)wParam);

          if (keyCode) w->onKeyDown(keyCode, flags);
          // w->onChar((char)wParam);
        } break;
        case WM_CHAR:
          w->onChar((char)wParam);
          break;

        case WM_KEYUP:
        case WM_SYSKEYUP: {
          unsigned long flags = 0;

          if (GetKeyState(VK_SHIFT) & 0x8000) {
            flags |= PX_MOD_SHIFT;
          }
          if (GetKeyState(VK_CONTROL) & 0x8000) {
            flags |= PX_MOD_CONTROL;
          }
          if (GetKeyState(VK_MENU) & 0x8000) {
            flags |= PX_MOD_ALT;
          }

          uint32_t keycode = keycodeFromNative((int)wParam);
          if (keycode) w->onKeyUp(keycode, flags);
        } break;
        case WM_PAINT:
#if 1
        {
          PAINTSTRUCT ps;
          HDC dc = BeginPaint(w->mWindow, &ps);
          w->onDraw(dc);
          SwapBuffers(
              dc);  // JRJR TODO should this be moved elsewhere into gl layer
          EndPaint(w->mWindow, &ps);
          return 0;
        }
#endif
        break;
        case WM_SIZE: {
          w->onSize(LOWORD(lParam), HIWORD(lParam));
        } break;
        case WM_ERASEBKGND: {
#if 0
                PAINTSTRUCT ps;
                HDC dc = BeginPaint(w->mWindow, &ps);
                w->onDraw(dc);
                EndPaint(w->mWindow, &ps);
#endif
        }
          return 0;
          break;
        case WM_SYNCHRONIZEDMESSAGE: {
          synchronizedMessage* m = (synchronizedMessage*)lParam;
          w->onSynchronizedMessage(m->messageName, m->p1);
        } break;
#endif
      }
    }
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}
