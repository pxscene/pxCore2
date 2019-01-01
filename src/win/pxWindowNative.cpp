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

// pxWindowNative.cpp

#define PX_NATIVE
#include "pxOffscreenNative.h"
#include "pxWindowNative.h"
#include "../pxWindow.h"
#include "../pxWindowUtil.h"

#include "../pxKeycodes.h"

#ifndef WINCE
#include <tchar.h>
#define _ATL_NO_HOSTING
//#include <atlconv.h>
#endif

#include "windowsx.h"

#pragma warning ( disable:4311)
#pragma warning ( disable:4312)

#define WM_DEFERREDCREATE           WM_USER+1000
// With the addition of getting native events
// maybe this should be deprecated?
#define WM_SYNCHRONIZEDMESSAGE      WM_USER+1001

#ifdef WINCE
#define MOBILE
#include "aygshell.h"
#endif

using namespace std;

void setWindowPtr(HWND w, void* p)
{
#ifndef WINCE
    SetWindowLongPtr(w, GWLP_USERDATA, (LONG)p);
#else
    SetWindowLong(w, GWL_USERDATA, (LONG)p);
#endif
}

void* getWindowPtr(HWND w)
{
#ifndef WINCE
    return (void*)GetWindowLongPtr(w, GWLP_USERDATA);
#else
    return (void*)GetWindowLong(w, GWL_USERDATA);
#endif
}


// pxWindow

pxError pxWindow::init(int left, int top, int width, int height)
{
#ifndef MOBILE
    return initNative(NULL, left, top, width, height, WS_OVERLAPPEDWINDOW, 0);
#else
    return initNative(NULL, left, top, width, height, WS_VISIBLE, 0);
#endif
}

#include <string>

pxError pxWindowNative::initNative(HWND parent, int left, int top, int width, int height, DWORD style, DWORD styleEx)
{
    HINSTANCE hInstance = ::GetModuleHandle(NULL);

    std::string className = "pxWindow";
    WNDCLASS wc;
    if (!::GetClassInfo(hInstance, className.c_str(), &wc))
    {

	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = (WNDPROC)windowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = className.c_str();

	RegisterClass(&wc);
    }

#ifndef MOBILE
    mWindow = ::CreateWindowEx(styleEx, "pxWindow", "", style, left, top, width, height, parent, NULL,
        hInstance, (pxWindowNative*)this);
#else
    mWindow = CreateWindow(className.c_str(), L"", style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, (pxWindowNative*)this);

    if (mWindow)
    {
        SHDoneButton (mWindow, SHDB_SHOW);
        SHFullScreen(mWindow, SHFS_HIDESIPBUTTON);
    }
#endif
    return mWindow?PX_OK:PX_FAIL;
}


void pxWindowNative::size(int& width, int& height)
{
    RECT r;
    GetClientRect(mWindow, &r);
    width = r.right-r.left;
    height = r.bottom-r.top;
}

pxError pxWindow::term()
{
//    ::DestroyWindow(mWindow);
    return PX_OK;
}

#if 1
void pxWindow::invalidateRect(pxRect* r)
{
    RECT wr;
    RECT* pwr = NULL;
    if (r)
    {
        pwr = &wr;
        SetRect(pwr, r->left(), r->top(), r->right(), r->bottom());
    }
    InvalidateRect(mWindow, pwr, FALSE);
}
#endif

bool pxWindow::visibility()
{
    return IsWindowVisible(mWindow)?true:false;
}

void pxWindow::setVisibility(bool visible)
{
    ShowWindow(mWindow, visible?SW_SHOW:SW_HIDE);
}

pxError pxWindow::setAnimationFPS(uint32_t fps)
{
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

pxError pxWindowNative::setAnimationFPS(long fps)
{
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
}

void pxWindow::setTitle(const char* title)
{
#if 0
	USES_CONVERSION;
    ::SetWindowText(mWindow, A2T(title));
#else
#ifndef WINCE
	::SetWindowTextA(mWindow, title);
#endif
#endif
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& s)
{
    s = ::GetDC(mWindow);
    return s?PX_OK:PX_FAIL;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& s)
{
    ::ReleaseDC(mWindow, s);
    return PX_OK;
}

// pxWindowNative

pxWindowNative::~pxWindowNative()
{
    setWindowPtr(mWindow, NULL);
}

void pxWindowNative::sendSynchronizedMessage(char* messageName, void *p1)
{
    synchronizedMessage m;
    m.messageName = messageName;
    m.p1 = p1;
    ::SendMessage(mWindow, WM_SYNCHRONIZEDMESSAGE, 0, (LPARAM)&m);
}

LRESULT __stdcall pxWindowNative::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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
        CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
        // ::SetProp(hWnd, _T("wnWindow"), (HANDLE)cs->lpCreateParams);
        setWindowPtr(hWnd, cs->lpCreateParams);
        w = (pxWindowNative*)cs->lpCreateParams;
        w->mWindow = hWnd;
    }
    else
        w = (pxWindowNative*)getWindowPtr(hWnd);


    bool consumed = false;

    if (w)
    {
        pxEventNativeDesc e;

        e.wnd = hWnd;
        e.msg = msg;
        e.wParam = wParam;
        e.lParam = lParam;

        w->onEvent(e, consumed);

        if (consumed)
		{
			// ick
			if (e.msg == WM_SETCURSOR)
				return 1;
			else
				return 0;
		}

		// re resolve the window ptr since we have destroyed it

        w = (pxWindowNative*)getWindowPtr(hWnd);
		if (w)
		{
        switch(msg)
        {
// Special case code to handle the "fake" close button in the form
// of an OK button on win mobile
#ifdef MOBILE
        case WM_COMMAND:
			{
            unsigned int wmId    = LOWORD(wParam); 
            unsigned int wmEvent = HIWORD(wParam); 
            // Parse the menu selections:
            switch (wmId)
            {
                case IDOK:
                    SendMessage (w->mWindow, WM_CLOSE, 0, 0);				
                    break;
                default:
                    return DefWindowProc(w->mWindow, msg, wParam, lParam);
            }
			}
            break;
#endif
#if 1
        case WM_CREATE:
            // We have to retry setting the animation timer if it happened to early
            // after creation
            w->onCreate();
            PostMessage(hWnd, WM_DEFERREDCREATE, 0, 0);
            break;
        case WM_DEFERREDCREATE:
            //w->onCreate();
            if (w->mAnimationFPS)
                w->setAnimationFPS(w->mAnimationFPS);
            break;

        case WM_CLOSE:
            w->onCloseRequest(); 
            return 0;
            break;

        case WM_DESTROY:
            w->onClose(); 
            //SetProp(hWnd, _T("wnWindow"), NULL);

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

        case WM_RBUTTONDOWN: mouseButtonShift++;
        case WM_MBUTTONDOWN: mouseButtonShift++;
        case WM_LBUTTONDOWN:
            {
#if 1
				SetCapture(w->mWindow);
                unsigned long flags = 1 << mouseButtonShift;
                if (wParam & MK_CONTROL) flags |= PX_MOD_CONTROL;
                if (wParam & MK_SHIFT) flags |= PX_MOD_SHIFT;
                if (GetKeyState(VK_MENU) < 0) flags |= PX_MOD_ALT;

                w->onMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), flags);

#endif
            }
            break;
        case WM_RBUTTONUP: mouseButtonShift++;
        case WM_MBUTTONUP: mouseButtonShift++;
        case WM_LBUTTONUP:
            {
#if 1
                ReleaseCapture();
                unsigned long flags = 1 << mouseButtonShift;
                if (wParam & MK_CONTROL) flags |= PX_MOD_CONTROL;
                if (wParam & MK_SHIFT) flags |= PX_MOD_SHIFT;
                if (GetKeyState(VK_MENU) < 0) flags |= PX_MOD_ALT;

                w->onMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), flags);
#endif
            }
            break;

        case WM_MOUSEWHEEL:
            {
                //int dx = (GET_X_LPARAM(lParam) > 0 ? 1 : -1);
                //int dy = (GET_Y_LPARAM(lParam) > 0 ? 1 : -1);
                GET_WHEEL_DELTA_WPARAM(wParam);
                int direction = (int(wParam) > 0) ? 1 : -1;

                w->onScrollWheel((float)(GET_X_LPARAM(lParam) * direction),  (float)direction);
	    }
	    break;
 
        case WM_MOUSEMOVE:
#if 1
#ifndef WINCE
			if (!w->mTrackMouse)
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;

				if (TrackMouseEvent(&tme))
				{
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
        case WM_SYSKEYDOWN:
            {
                unsigned long flags = 0;

                if (GetKeyState(VK_SHIFT) & 0x8000)
                {
                    flags |= PX_MOD_SHIFT;
                }
                if (GetKeyState(VK_CONTROL) & 0x8000)
                {
                    flags |= PX_MOD_CONTROL;
                }
                if (GetKeyState(VK_MENU) & 0x8000)
                {
                    flags |= PX_MOD_ALT;
                }

                w->onKeyDown(keycodeFromNative((int)wParam), flags);
                //w->onChar((char)wParam);
            }
            break;
				case WM_CHAR: 
					w->onChar((char)wParam);
					break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            {
                unsigned long flags = 0;

                if (GetKeyState(VK_SHIFT) & 0x8000)
                {
                    flags |= PX_MOD_SHIFT;
                }
                if (GetKeyState(VK_CONTROL) & 0x8000)
                {
                    flags |= PX_MOD_CONTROL;
                }
                if (GetKeyState(VK_MENU) & 0x8000)
                {
                    flags |= PX_MOD_ALT;
                }

                w->onKeyUp(keycodeFromNative((int)wParam), flags);
            }
            break;
        case WM_PAINT:
#if 1
            {
                PAINTSTRUCT ps;
                HDC dc = BeginPaint(w->mWindow, &ps);
                w->onDraw(dc);
								SwapBuffers(dc);
                EndPaint(w->mWindow, &ps);
				return 0;
            }
#endif
            break;
        case WM_SIZE:
            {
                w->onSize(LOWORD(lParam), HIWORD(lParam));
            }
            break;
        case WM_ERASEBKGND:
            {
#if 0
                PAINTSTRUCT ps;
                HDC dc = BeginPaint(w->mWindow, &ps);
                w->onDraw(dc);
                EndPaint(w->mWindow, &ps);
#endif
            }
            return 0;
            break;
        case WM_SYNCHRONIZEDMESSAGE:
            {
                synchronizedMessage *m = (synchronizedMessage*)lParam;
                w->onSynchronizedMessage(m->messageName, m->p1);
            }
            break;
#endif
        }
		}
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
