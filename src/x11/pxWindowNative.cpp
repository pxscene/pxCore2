// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxWindowNative.cpp

#include "../pxCore.h"
#include "../pxWindow.h"
#include "pxWindowNative.h"
#include "../pxTimer.h"
#include "../pxWindowUtil.h"

#include <stdlib.h>
#include <string.h>

Display* displayRef::mDisplay = NULL;
int displayRef::mRefCount = 0;

bool exitFlag = false;

// pxWindow

pxError pxWindow::init(int left, int top, int width, int height)
{
    Window rootwin;

    int scr;
    Display* dpy = mDisplayRef.getDisplay();

    scr = DefaultScreen(dpy);
    rootwin = RootWindow(dpy, scr);
    
    win=XCreateSimpleWindow(dpy, rootwin, left, top, width, height, 0, 
                            BlackPixel(dpy, scr), BlackPixel(dpy, scr));

    if (win)
    {
	XSizeHints      size_hints ;
    
	size_hints.flags = PPosition ;
	size_hints.x = left ;
	size_hints.y = top ;
	XSetNormalHints( dpy, win, &size_hints) ; 
    
	XWindowAttributes attr;
	XGetWindowAttributes(dpy, win, &attr);

	this->onSize(attr.width, attr.height);
    
	XSelectInput(dpy, win, 
                 PointerMotionMask|
                 ExposureMask|
                 ButtonPressMask|ButtonReleaseMask|
                 KeyPressMask|KeyReleaseMask |
                 StructureNotifyMask | LeaveWindowMask
	    );
	
	registerWindow(win, this);
    
	XSetWindowBackgroundPixmap(dpy, win, None);
    
	closeatom = XInternAtom(dpy, 
				"WM_DELETE_WINDOW", 
				false);
    
	XSetWMProtocols(dpy, 
			win, 
			&closeatom, 
			1);
    
	this->onCreate();
    }
    
    return win?PX_OK:PX_FAIL;
}

pxError pxWindow::term()
{
    XDestroyWindow(mDisplayRef.getDisplay(), win);
    return PX_OK;
}

void pxWindow::invalidateRect(pxRect *r)
{
    invalidateRectInternal(r);
}

// This can be improved by collecting the dirty regions and painting
// when the event loop goes idle
void pxWindowNative::invalidateRectInternal(pxRect *r)
{
    Display* display = mDisplayRef.getDisplay();
    GC gc=XCreateGC(display, win, 0, NULL);
                
    pxSurfaceNativeDesc d;
    d.display = display;
    d.drawable = win;
    d.gc = gc;
    
    if (r)
    {
	// Set up clip area
	XRectangle xr;
	xr.x = r->left();
	xr.y = r->top();
	xr.width = r->width();
	xr.height = r->height();
	XSetClipRectangles(display, gc, 0, 0, &xr, 1, Unsorted);
    }

    onDraw(&d);
    
    XFreeGC(display, gc);

}

bool pxWindow::visibility()
{
    XWindowAttributes attr;
    XGetWindowAttributes(mDisplayRef.getDisplay(), win, &attr);

    //    printf("mapstate %d\n", attr.map_state);

    return (attr.map_state == IsViewable);
}

void pxWindow::setVisibility(bool visible)
{
    Display* d = mDisplayRef.getDisplay();
    if (!visible)
        XUnmapWindow(d, win);
    else
    {
        XMapWindow(d, win);
    }
}

pxError pxWindow::setAnimationFPS(long fps)
{
    mTimerFPS = fps;
    mLastAnimationTime = pxMilliseconds();
    return PX_OK;
}

void pxWindow::setTitle(char* title)
{
    Display* d = mDisplayRef.getDisplay();
    XTextProperty tp;
    tp.value = (unsigned char *)title;
    tp.encoding = XA_WM_NAME;
    tp.format = 8; // 8 bit chars
    tp.nitems = strlen(title);

    XSetWMName(d, win, &tp);
    XStoreName(d, win, title);
    XSetWMIconName(d, win, &tp);
    XSetIconName(d, win, title);
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& s)
{
    s = (pxSurfaceNative)malloc(sizeof(pxSurfaceNativeDesc));
    s->display = mDisplayRef.getDisplay();
    s->drawable = win;
    s->gc = XCreateGC(s->display, win, 0, NULL);

    return PX_OK;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& s)
{
    XFreeGC(s->display, s->gc);
    free(s);
    s = NULL;

    return PX_OK;
}

// pxWindowNative

void pxWindowNative::onAnimationTimerInternal()
{
    if (mTimerFPS) onAnimationTimer();
}

void pxWindowNative::runEventLoop()
{
    displayRef d;
        
    exitFlag = false;

    double lastAnimationTime = pxMilliseconds();
    while(!exitFlag)
    {
	
        XEvent e;
        if (XPending(d.getDisplay()))
        {
	    XNextEvent(d.getDisplay(), &e);
	    XAnyEvent* ae = (XAnyEvent*)&e;
	    
	    pxWindowNative* w = getPXWindowFromX11Window(ae->window);
	    if (w)
	    {
		switch(ae->type)
		{
		case Expose:
		{
		    if(e.xexpose.count<1)
		    {
		    
			GC gc=XCreateGC(ae->display, ae->window, 0, NULL);
			
			pxSurfaceNativeDesc d;
			d.display = ae->display;
			d.drawable = ae->window;
			d.gc = gc;
			
			w->onDraw(&d);
			
			XFreeGC(ae->display, gc);
		    }
		}
		break;
		
		case ButtonPress:
		{
		    
		    XGrabPointer(ae->display, ae->window, true, 
				 ButtonPressMask|ButtonReleaseMask|
				 PointerMotionMask,
				 GrabModeAsync, GrabModeAsync, None, None, 
				 CurrentTime);
		    
		    XButtonEvent *be = (XButtonEvent*)ae;
		    unsigned long flags;
		    switch(be->button)
		    {
		    case Button2: flags = PX_MIDDLEBUTTON;
			break;
		    case Button3: flags = PX_RIGHTBUTTON;
			break;
		    default: flags = PX_LEFTBUTTON;
			break;
		    }
		    flags |= (be->state & ShiftMask)?PX_MOD_SHIFT:0;
		    flags |= (be->state & ControlMask)?PX_MOD_CONTROL:0;
		    flags |= (be->state & Mod1Mask)?PX_MOD_ALT:0;
		    
		    w->onMouseDown(be->x, be->y, flags);
		}
		break;

		case ButtonRelease:
		{
		    XUngrabPointer(ae->display, CurrentTime);
		    
		    XButtonEvent *be = (XButtonEvent*)ae;
		    unsigned long flags;
		    switch(be->button)
		    {
		    case Button2: flags = PX_MIDDLEBUTTON;
			break;
		    case Button3: flags = PX_RIGHTBUTTON;
			break;
		    default: flags = PX_LEFTBUTTON;
			break;
		    }
		    flags |= (be->state & ShiftMask)?PX_MOD_SHIFT:0;
		    flags |= (be->state & ControlMask)?PX_MOD_CONTROL:0;
		    flags |= (be->state & Mod1Mask)?PX_MOD_ALT:0;
		    
		    w->onMouseUp(be->x, be->y, flags);
		}
		break;

		case KeyPress:
		{		
		    XKeyEvent* ke = (XKeyEvent*)ae;
		    KeySym keySym = ::XKeycodeToKeysym(ae->display, 
						       e.xkey.keycode, 
						       0);
		    if (keySym >= 'a' && keySym <= 'z')
			keySym = (keySym-'a')+'A';
		    else if (keySym == XK_Shift_R)
			keySym = XK_Shift_L;
		    else if (keySym == XK_Control_R)
			keySym = XK_Control_L;
		    else if (keySym == XK_Alt_R)
			keySym = XK_Alt_L;
		    
		    unsigned long flags = 0;
		    flags |= (ke->state & ShiftMask)?PX_MOD_SHIFT:0;
		    flags |= (ke->state & ControlMask)?PX_MOD_CONTROL:0;
		    flags |= (ke->state & Mod1Mask)?PX_MOD_ALT:0;
        w->onKeyDown(keycodeFromNative(keySym), flags);
        w->onChar((char)keySym);
		}
		break;

		case MotionNotify:
		{
		    XMotionEvent *me = (XMotionEvent*)ae;
		    w->onMouseMove(me->x, me->y);
		}
		break;

		case KeyRelease:
		{
		    XKeyEvent* ke = (XKeyEvent*)ae;
		    KeySym keySym = ::XKeycodeToKeysym(ae->display, 
						       e.xkey.keycode, 
						       0);
		    
		    if (keySym >= 'a' && keySym <= 'z')
			keySym = (keySym-'a')+'A';
		    else if (keySym == XK_Shift_R)
			keySym = XK_Shift_L;
		    else if (keySym == XK_Control_R)
			keySym = XK_Control_L;
		    else if (keySym == XK_Alt_R)
			keySym = XK_Alt_L;
		    
		    unsigned long flags = 0;
		    flags |= (ke->state & ShiftMask)?PX_MOD_SHIFT:0;
		    flags |= (ke->state & ControlMask)?PX_MOD_CONTROL:0;
		    flags |= (ke->state & Mod1Mask)?PX_MOD_ALT:0;
		    w->onKeyUp(keySym, flags);
		}
		break;

		case ConfigureNotify:
		{
		    // We defer the onSize message after some
		    // time
		    if (w->lastWidth != e.xconfigure.width ||
			w->lastHeight != e.xconfigure.height)
		    {
			w->resizeFlag = true;
			w->lastWidth = e.xconfigure.width;
			w->lastHeight = e.xconfigure.height;
		    }
		}
		break;

		case ClientMessage:
		{
		    
		    if((e.xclient.format == 32) &&
		       (e.xclient.data.l[0] == int(w->closeatom)))
		    {
			w->onCloseRequest();
		    }
		}
		break;

		case DestroyNotify:
		{
		    w->onClose();
		    unregisterWindow(ae->window);
		}
		break;
		case LeaveNotify:
		  {
		    w->onMouseLeave();
		  }
		  break;
		}
	    }
        }
        else
        {
	    // The animation/resize handling under x11 needs some serious
	    // rework
	    
            double currentAnimationTime = pxMilliseconds();
	    
	    vector<windowDesc>::iterator i;
	    for (i = mWindowMap.begin(); i < mWindowMap.end(); i++)
	    {
		pxWindowNative* w = (*i).p;

		double animationDelta = currentAnimationTime-lastAnimationTime;
		if (w->resizeFlag)
		{
		    w->resizeFlag = false;
		    w->onSize((*i).p->lastWidth, (*i).p->lastHeight);
		    w->invalidateRectInternal(NULL);
		}
		
		if (w->mTimerFPS)
		{
		    animationDelta = currentAnimationTime-
			w->mLastAnimationTime;

		    if (animationDelta > (1000/w->mTimerFPS))
		    {
			w->onAnimationTimerInternal();
			w->mLastAnimationTime = currentAnimationTime;
		    }
		}
	    }

	    pxSleepMS(10); // Breath
        }
    }
}

void pxWindowNative::exitEventLoop()
{
    exitFlag = true;
}


void pxWindowNative::registerWindow(Window w, pxWindowNative* p)
{
    windowDesc d = {w, p};
    mWindowMap.push_back(d);
}

void pxWindowNative::unregisterWindow(Window w)
{
    vector<windowDesc>::iterator i;

    for (i = mWindowMap.begin(); i < mWindowMap.end(); i++)
    {
        if ((*i).w == w) 
        {
            mWindowMap.erase(i);
            return;
        }
    }
}

pxWindowNative* pxWindowNative::getPXWindowFromX11Window(Window w)
{
    vector<windowDesc>::iterator i;

    for (i = mWindowMap.begin(); i < mWindowMap.end(); i++)
    {
        if ((*i).w == w) 
            return (*i).p;
    }
    return NULL;
}

vector<pxWindowNative::windowDesc> pxWindowNative::mWindowMap;
