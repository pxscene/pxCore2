#ifndef PX_VIEWWINDOW_H
#define PX_VIEWWINDOW_H

#include "pxWindow.h"
#include "pxIView.h"

class pxViewWindow: public pxWindow, public pxIViewListener
{
public:

    // pxViewRef is a smart ptr
    // that manages the refcount on a pxIView instance
    pxError view(pxViewRef& v);
    // Specifying NULL will release the view
    pxError setView(pxIView* v);

protected:

    virtual void onClose();

    virtual void RT_STDCALL invalidateRect(pxRect* r);

    // The following methods are delegated to the view
    virtual void onSize(int w, int h);
    virtual void onMouseDown(int x, int y, unsigned long flags);
    virtual void onMouseUp(int x, int y, unsigned long flags);
    virtual void onMouseLeave();
    virtual void onMouseMove(int x, int y);

#if 0
    virtual void onKeyDown(int keycode, unsigned long flags);
    virtual void onKeyUp(int keycode, unsigned long flags);
#endif

    virtual void onDraw(pxSurfaceNative s);

private:
    pxViewRef mView;
    pxOffscreen mViewOffscreen;
};

#endif // PX_VIEWWINDOW_H
