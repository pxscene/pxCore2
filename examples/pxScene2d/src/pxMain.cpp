// pxCore CopyRight 2007-2015 John Robinson
// main.cpp

#ifdef WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxOffscreen.h"
#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#ifdef ENABLE_GLUT
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <GLES2/gl2.h>
#endif
#endif

#include "rtLog.h"
#include "rtRefT.h"
#include "rtPathUtils.h"
#include "pxScene2d.h"
#include "pxImage.h"
#include "pxText.h"

pxScene2d scene;

void initGL();

#if 0
void setupScene() {

  rtString d;
  rtGetCurrentDirectory(d);
  d.append("/../images/grapes.png");

  pxObjectRef root = scene.getRoot();
  
  int n = 3;
  int nx = 100;
  int ny = 200;
  for (int i = 0; i < n; i++) {
    pxObjectRef p;

    if (i < 2) {
      pxText* t = new pxText();
      p = t;
      char buffer[256];
      sprintf(buffer, "Hello %d", i);
      t->setText(buffer);
    }
    else {
      pxImage* i = new pxImage();
      p = i;
      i->setURL(d.cString());
      p->cx = i->width()/2;
      p->cy = i->height()/2;
    }

    nx += 10;
    if (nx > 1000) {
      nx = 0;
      ny += 10;
    }
    p->x = nx;
    p->y = ny;

    p->setParent(root);
    
    p->r = 45*i;
    p->animateTo("r", 360+(90*i), 2.0, pxInterpLinear, seesaw);
    p->animateTo("x", 800, 2.0, pxInterpLinear, seesaw);
  }
}

#else
void setupScene() {
  rtString d;
  rtGetCurrentDirectory(d);
  d.append("/../images/banana.png");

  pxObjectRef root = scene.getRoot();
  
  int n = 3;
  int nx = 100;
  int ny = 200;
  for (int i = 0; i < n; i++) {
    pxObjectRef p;

    if (i == 0) {
      rtRefT<rectangle9> r = new rectangle9();
      float c1[4] = {1,0,0,1};
      r->setFillColor(c1);
      float c2[4] = {1,1,1,0.5};
      r->setLineColor(c2);
      r->setLineWidth(10);
      r->w = 300;
      r->h = 30;
      p = r;
      p->animateTo("h", 600, 0.2, pxInterpLinear, seesaw);
    }
    else if (i == 1) {
      rtRefT<pxText> t = new pxText();
      t->setText("Hello");
      p = t;
    }
    else {
      pxImage* i = new pxImage();
      p = i;
      i->setURL(d.cString());
      p->cx = i->width()/2;
      p->cy = i->height()/2;
    }

    nx += 10;
    if (nx > 1000) {
      nx = 0;
      ny += 10;
    }
    p->x = nx;
    p->y = ny;

    p->setParent(root);
    
    p->animateTo("r", 360, 2.0, pxInterpLinear, loop);
    p->animateTo("x", 800, 2.0, pxInterpLinear, seesaw);
    p->animateTo("a", 0.5, 2.0, pxInterpLinear, seesaw);
  }
}

#endif

pxEventLoop eventLoop;

class myWindow: public pxWindow
{
private:
    // Event Handlers - Look in pxWindow.h for more
    void onCloseRequest()
    {
        // When someone clicks the close box no policy is predefined.
        // so we need to explicitly tell the event loop to exit
	eventLoop.exit();
    }

    void onSize(int newWidth, int newHeight)
    {
        scene.onSize(newWidth, newHeight);
    }

    void onDraw(pxSurfaceNative s)
    {
        scene.onDraw();
    }
    
    void onMouseDown(int x, int y, unsigned long flags)
    {
        printf("Mouse Down (%d %d) modifiers: [", x, y);
        if (flags & PX_LEFTBUTTON) printf("Left ");
        if (flags & PX_MIDDLEBUTTON) printf("Middle ");
        if (flags & PX_RIGHTBUTTON) printf("Right ");
        if (flags & PX_MOD_SHIFT) printf("Shift ");
        if (flags & PX_MOD_CONTROL) printf("Control ");
        if (flags & PX_MOD_ALT) printf("Alt ");
        printf("]\n");
    }

    void onMouseUp(int x, int y, unsigned long flags)
    {
        printf("Mouse Up (%d, %d) modifiers: [ ", x, y);
        if (flags & PX_LEFTBUTTON) printf("Left ");
        if (flags & PX_MIDDLEBUTTON) printf("Middle ");
        if (flags & PX_RIGHTBUTTON) printf("Right ");
        if (flags & PX_MOD_SHIFT) printf("Shift ");
        if (flags & PX_MOD_CONTROL) printf("Control ");
        if (flags & PX_MOD_ALT) printf("Alt ");
        printf("]\n");
    }

    void onMouseMove(int x, int y)
    {
        //printf("Mouse Move %d, %d\n", x, y);
        scene.onMouseMove(x,y);
    }

    void onKeyDown(int c, unsigned long flags)
    {
        printf("Key Dn \"%s\" modifiers: [", getKeyDescription(c));
        if (c == PX_KEY_ESCAPE)
        {
            exit(0);
            return;
        }
        if (flags & PX_MOD_SHIFT) printf("Shift ");
        if (flags & PX_MOD_CONTROL) printf("Control ");
        if (flags & PX_MOD_ALT) printf("Alt ");
		printf("]");
		printf(" Keycode: 0x%x", c);
		printf("\n");
    }

    void onKeyUp(int c, unsigned long flags)
    {
        printf("Key Up \"%s\" modifiers: [", getKeyDescription(c));
        if (flags & PX_MOD_SHIFT) printf("Shift ");
        if (flags & PX_MOD_CONTROL) printf("Control ");
        if (flags & PX_MOD_ALT) printf("Alt ");
		printf("]");
		printf(" Keycode: 0x%x", c);
		printf("\n");
    }

    const char * getKeyDescription( int keycode )
    {
        switch (keycode)
        {
            case PX_KEY_PAUSE: return "Pause";
            case PX_KEY_ENTER: return "Enter";
            case PX_KEY_BACKSPACE: return "BackSpace";
            case PX_KEY_TAB: return "Tab";
            case PX_KEY_SHIFT: return "Shift";
            case PX_KEY_CONTROL: return "Control";
			case PX_KEY_ALT: return "Alt";
			case PX_KEY_CAPSLOCK: return "CapsLock";
            case PX_KEY_ESCAPE: return "Escape";
            case PX_KEY_SPACE: return "Space";
            case PX_KEY_PAGEUP: return "PageUp";
            case PX_KEY_PAGEDOWN: return "PageDown";
            case PX_KEY_END: return "End";
            case PX_KEY_HOME: return "Home";
            case PX_KEY_LEFT: return "Left";
            case PX_KEY_UP: return "Up";
            case PX_KEY_RIGHT: return "Right";
            case PX_KEY_DOWN: return "Down";
            case PX_KEY_COMMA: return "Comma";
            case PX_KEY_PERIOD: return "Period";
            case PX_KEY_SLASH: return "Slash";
            case PX_KEY_ZERO: return "Zero";
            case PX_KEY_ONE: return "One";
            case PX_KEY_TWO: return "Two";
            case PX_KEY_THREE: return "Three";
            case PX_KEY_FOUR: return "Four";
            case PX_KEY_FIVE: return "Five";
            case PX_KEY_SIX: return "Six";
            case PX_KEY_SEVEN: return "Seven";
            case PX_KEY_EIGHT: return "Eight";
            case PX_KEY_NINE: return "Nine";
            case PX_KEY_SEMICOLON: return "SemiColon";
            case PX_KEY_EQUALS: return "Equals";
            case PX_KEY_A: return "A";
            case PX_KEY_B: return "B";
            case PX_KEY_C: return "C";
            case PX_KEY_D: return "D";
            case PX_KEY_E: return "E";
            case PX_KEY_F: return "F";
            case PX_KEY_G: return "G";
            case PX_KEY_H: return "H";
            case PX_KEY_I: return "I";
            case PX_KEY_J: return "J";
            case PX_KEY_K: return "K";
            case PX_KEY_L: return "L";
            case PX_KEY_M: return "M";
            case PX_KEY_N: return "N";
            case PX_KEY_O: return "O";
            case PX_KEY_P: return "P";
            case PX_KEY_Q: return "Q";
            case PX_KEY_R: return "R";
            case PX_KEY_S: return "S";
            case PX_KEY_T: return "T";
            case PX_KEY_U: return "U";
            case PX_KEY_V: return "V";
            case PX_KEY_W: return "W";
            case PX_KEY_X: return "X";
            case PX_KEY_Y: return "Y";
            case PX_KEY_Z: return "Z";
            case PX_KEY_OPENBRACKET: return "OpenBracket";
            case PX_KEY_BACKSLASH: return "BackSlash";
            case PX_KEY_CLOSEBRACKET: return "CloseBracket";
            case PX_KEY_NUMPAD0: return "NumPad0";
            case PX_KEY_NUMPAD1: return "NumPad1";
            case PX_KEY_NUMPAD2: return "NumPad2";
            case PX_KEY_NUMPAD3: return "NumPad3";
            case PX_KEY_NUMPAD4: return "NumPad4";
            case PX_KEY_NUMPAD5: return "NumPad5";
            case PX_KEY_NUMPAD6: return "NumPad6";
            case PX_KEY_NUMPAD7: return "NumPad7";
            case PX_KEY_NUMPAD8: return "NumPad8";
            case PX_KEY_NUMPAD9: return "NumPad9";
            case PX_KEY_SEPARATOR: return "Separator";
            case PX_KEY_ADD: return "Add";
            case PX_KEY_SUBTRACT: return "Subtract";
            case PX_KEY_DECIMAL: return "Decimal";
            case PX_KEY_DIVIDE: return "Divide";
            case PX_KEY_MULTIPLY: return "Multiply";
            case PX_KEY_F1: return "F1";
            case PX_KEY_F2: return "F2";
            case PX_KEY_F3: return "F3";
            case PX_KEY_F4: return "F4";
            case PX_KEY_F5: return "F5";
            case PX_KEY_F6: return "F6";
            case PX_KEY_F7: return "F7";
            case PX_KEY_F8: return "F8";
            case PX_KEY_F9: return "F9";
            case PX_KEY_F10: return "F10";
            case PX_KEY_F11: return "F11";
            case PX_KEY_F12: return "F12";
            case PX_KEY_DELETE: return "Delete";
            case PX_KEY_NUMLOCK: return "NumLock";
            case PX_KEY_SCROLLLOCK: return "ScrollLock";
			case PX_KEY_PRINTSCREEN: return "PrintScreen";
            case PX_KEY_INSERT: return "Insert";
			case PX_KEY_BACKQUOTE: return "BackQuote";
			case PX_KEY_QUOTE: return "Quote";
            default: return "Undefined";
        }
    }
};

int pxMain()
{
    int width = 1280;
    int height = 720;
    myWindow win;

    win.init(10, 10, width, height);
    setupScene();
    initGL();
    scene.onSize(width, height);
    win.setTitle("TestGL");
    win.setVisibility(true);

    eventLoop.run();

    return 0;
}



