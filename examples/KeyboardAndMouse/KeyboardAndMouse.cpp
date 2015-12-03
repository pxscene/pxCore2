// Keyboard and Mouse Example CopyRight 2007-2015 John Robinson
// Demonstrates how to handle keyboard and mouse events

#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"

#include "pxOffscreen.h"

#include <stdio.h> // for printf

pxEventLoop eventLoop;

void drawBackground(pxBuffer& b)
{
  // Fill the buffer with a simple pattern as a function of f(x,y)
  int w = b.width();
  int h = b.height();

  for (int y = 0; y < h; y++)
  {
    pxPixel* p = b.scanline(y);
    for (int x = 0; x < w; x++)
    {
      p->r = pxClamp<int>(x+y, 255);
      p->g = pxClamp<int>(y,   255);
      p->b = pxClamp<int>(x,   255);
      p++;
    }
  }
}

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
    // When ever the window resizes (re)allocate a buffer
    // big enough for the entire
    // client area and draw our pattern into it
    mTexture.init(newWidth, newHeight);
    drawBackground(mTexture);
  }

  void onDraw(pxSurfaceNative s)
  {
    // Draw the texture into this window
    //mTexture.blit(s);
  }

  void onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    printf("Mouse Down (%d %d) modifiers: [", x, y);
    if (flags & PX_LEFTBUTTON)   printf("Left ");
    if (flags & PX_MIDDLEBUTTON) printf("Middle ");
    if (flags & PX_RIGHTBUTTON)  printf("Right ");
    if (flags & PX_MOD_SHIFT)    printf("Shift ");
    if (flags & PX_MOD_CONTROL)  printf("Control ");
    if (flags & PX_MOD_ALT)      printf("Alt ");
    printf("]\n");
  }

  void onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    printf("Mouse Up (%d, %d) modifiers: [ ", x, y);
    if (flags & PX_LEFTBUTTON)   printf("Left ");
    if (flags & PX_MIDDLEBUTTON) printf("Middle ");
    if (flags & PX_RIGHTBUTTON)  printf("Right ");
    if (flags & PX_MOD_SHIFT)    printf("Shift ");
    if (flags & PX_MOD_CONTROL)  printf("Control ");
    if (flags & PX_MOD_ALT)      printf("Alt ");
    printf("]\n");
  }

  void onMouseMove(int32_t x, int32_t y)
  {
    printf("Mouse Move %d, %d\n", x, y);
  }

  void onKeyDown(uint32_t c, uint32_t flags)
  {
    printf("Key Dn \"%s\" modifiers: [", getKeyDescription(c));
    if (flags & PX_MOD_SHIFT)   printf("Shift ");
    if (flags & PX_MOD_CONTROL) printf("Control ");
    if (flags & PX_MOD_ALT)     printf("Alt ");
    if (flags & PX_MOD_COMMAND)     printf("Command ");
    printf("]");
    printf(" Keycode: 0x%x", c);
    printf("\n");
  }

  void onKeyUp(uint32_t c, uint32_t flags)
  {
    printf("Key Up \"%s\" modifiers: [", getKeyDescription(c));
    if (flags & PX_MOD_SHIFT)   printf("Shift ");
    if (flags & PX_MOD_CONTROL) printf("Control ");
    if (flags & PX_MOD_ALT)     printf("Alt ");
    if (flags & PX_MOD_COMMAND)     printf("Command ");    
    printf("]");
    printf(" Keycode: 0x%x", c);
    printf("\n");
  }
  
  void onChar(uint32_t c)
  {
    printf("Char \"%c\"; code: %d\n", c, c);
  }

  const char* getKeyDescription( uint32_t keycode )
  {
    switch (keycode)
    {
#if 1

    case PX_KEY_NATIVE_PAUSE:     return "Pause";
    case PX_KEY_NATIVE_ENTER:     return "Enter";
    case PX_KEY_NATIVE_BACKSPACE: return "BackSpace";
    case PX_KEY_NATIVE_TAB:       return "Tab";
    case PX_KEY_NATIVE_SHIFT:     return "Shift";
    case PX_KEY_NATIVE_CONTROL:   return "Control";
    case PX_KEY_NATIVE_ALT:       return "Alt";
    case PX_KEY_NATIVE_CAPSLOCK:  return "CapsLock";
    case PX_KEY_NATIVE_ESCAPE:    return "Escape";
    case PX_KEY_NATIVE_SPACE:     return "Space";
    case PX_KEY_NATIVE_PAGEUP:    return "PageUp";
    case PX_KEY_NATIVE_PAGEDOWN:  return "PageDown";
    case PX_KEY_NATIVE_END:       return "End";
    case PX_KEY_NATIVE_HOME:      return "Home";
    case PX_KEY_NATIVE_LEFT:      return "Left";
    case PX_KEY_NATIVE_UP:        return "Up";
    case PX_KEY_NATIVE_RIGHT:     return "Right";
    case PX_KEY_NATIVE_DOWN:      return "Down";
    case PX_KEY_NATIVE_COMMA:     return "Comma";
    case PX_KEY_NATIVE_PERIOD:    return "Period";
    case PX_KEY_NATIVE_SLASH:     return "Slash";
    case PX_KEY_NATIVE_ZERO:      return "Zero";
    case PX_KEY_NATIVE_ONE:       return "One";
    case PX_KEY_NATIVE_TWO:       return "Two";
    case PX_KEY_NATIVE_THREE:     return "Three";
    case PX_KEY_NATIVE_FOUR:      return "Four";
    case PX_KEY_NATIVE_FIVE:      return "Five";
    case PX_KEY_NATIVE_SIX:       return "Six";
    case PX_KEY_NATIVE_SEVEN:     return "Seven";
    case PX_KEY_NATIVE_EIGHT:     return "Eight";
    case PX_KEY_NATIVE_NINE:      return "Nine";
    case PX_KEY_NATIVE_SEMICOLON: return "SemiColon";
    case PX_KEY_NATIVE_EQUALS:    return "Equals";
    case PX_KEY_NATIVE_A: return "A";
    case PX_KEY_NATIVE_B: return "B";
    case PX_KEY_NATIVE_C: return "C";
    case PX_KEY_NATIVE_D: return "D";
    case PX_KEY_NATIVE_E: return "E";
    case PX_KEY_NATIVE_F: return "F";
    case PX_KEY_NATIVE_G: return "G";
    case PX_KEY_NATIVE_H: return "H";
    case PX_KEY_NATIVE_I: return "I";
    case PX_KEY_NATIVE_J: return "J";
    case PX_KEY_NATIVE_K: return "K";
    case PX_KEY_NATIVE_L: return "L";
    case PX_KEY_NATIVE_M: return "M";
    case PX_KEY_NATIVE_N: return "N";
    case PX_KEY_NATIVE_O: return "O";
    case PX_KEY_NATIVE_P: return "P";
    case PX_KEY_NATIVE_Q: return "Q";
    case PX_KEY_NATIVE_R: return "R";
    case PX_KEY_NATIVE_S: return "S";
    case PX_KEY_NATIVE_T: return "T";
    case PX_KEY_NATIVE_U: return "U";
    case PX_KEY_NATIVE_V: return "V";
    case PX_KEY_NATIVE_W: return "W";
    case PX_KEY_NATIVE_X: return "X";
    case PX_KEY_NATIVE_Y: return "Y";
    case PX_KEY_NATIVE_Z: return "Z";
    case PX_KEY_NATIVE_OPENBRACKET:  return "OpenBracket";
    case PX_KEY_NATIVE_BACKSLASH:    return "BackSlash";
    case PX_KEY_NATIVE_CLOSEBRACKET: return "CloseBracket";
    case PX_KEY_NATIVE_NUMPAD0: return "NumPad0";
    case PX_KEY_NATIVE_NUMPAD1: return "NumPad1";
    case PX_KEY_NATIVE_NUMPAD2: return "NumPad2";
    case PX_KEY_NATIVE_NUMPAD3: return "NumPad3";
    case PX_KEY_NATIVE_NUMPAD4: return "NumPad4";
    case PX_KEY_NATIVE_NUMPAD5: return "NumPad5";
    case PX_KEY_NATIVE_NUMPAD6: return "NumPad6";
    case PX_KEY_NATIVE_NUMPAD7: return "NumPad7";
    case PX_KEY_NATIVE_NUMPAD8: return "NumPad8";
    case PX_KEY_NATIVE_NUMPAD9: return "NumPad9";
    case PX_KEY_NATIVE_SEPARATOR: return "Separator";
    case PX_KEY_NATIVE_ADD:       return "Add";
    case PX_KEY_NATIVE_SUBTRACT:  return "Subtract";
    case PX_KEY_NATIVE_DECIMAL:   return "Decimal";
    case PX_KEY_NATIVE_DIVIDE:    return "Divide";
    case PX_KEY_NATIVE_MULTIPLY:  return "Multiply";
    case PX_KEY_NATIVE_F1: return "F1";
    case PX_KEY_NATIVE_F2: return "F2";
    case PX_KEY_NATIVE_F3: return "F3";
    case PX_KEY_NATIVE_F4: return "F4";
    case PX_KEY_NATIVE_F5: return "F5";
    case PX_KEY_NATIVE_F6: return "F6";
    case PX_KEY_NATIVE_F7: return "F7";
    case PX_KEY_NATIVE_F8: return "F8";
    case PX_KEY_NATIVE_F9: return "F9";
    case PX_KEY_NATIVE_F10: return "F10";
    case PX_KEY_NATIVE_F11: return "F11";
    case PX_KEY_NATIVE_F12: return "F12";
    case PX_KEY_NATIVE_DELETE:      return "Delete";
    case PX_KEY_NATIVE_NUMLOCK:     return "NumLock";
    case PX_KEY_NATIVE_SCROLLLOCK:  return "ScrollLock";
    case PX_KEY_NATIVE_PRINTSCREEN: return "PrintScreen";
    case PX_KEY_NATIVE_INSERT:      return "Insert";
    case PX_KEY_NATIVE_BACKQUOTE:   return "BackQuote";
    case PX_KEY_NATIVE_QUOTE:       return "Quote";
#endif
    default: return "Undefined";
    }
  }

  pxOffscreen mTexture;
};

int pxMain()
{
  myWindow win;

  win.init(10, 64, 300, 240);
  win.setTitle("Keyboard And Mouse");
  win.setVisibility(true);

  eventLoop.run();

  return 0;
}


