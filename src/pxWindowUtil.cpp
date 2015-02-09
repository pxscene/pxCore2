#include <stdio.h>

#include "pxWindowUtil.h"
#include "pxCore.h"
#include "pxKeycodes.h"

int keycodeFromNative(int nativeKeycode)
{
  int commonKeycode = 0;
  switch (nativeKeycode)
  {
    case PX_KEY_NATIVE_ENTER:
      commonKeycode = PX_KEY_ENTER;
      break;
    case PX_KEY_NATIVE_BACKSPACE:
      commonKeycode = PX_KEY_BACKSPACE;
      break;
    case PX_KEY_NATIVE_TAB:
      commonKeycode = PX_KEY_TAB;
      break;
    case PX_KEY_NATIVE_CANCEL:
      commonKeycode = PX_KEY_NATIVE_CANCEL; //TODO
      break;
    case PX_KEY_NATIVE_CLEAR:
      commonKeycode = PX_KEY_NATIVE_CLEAR; //TODO
      break;
    case PX_KEY_NATIVE_SHIFT:
      commonKeycode = PX_KEY_SHIFT;
      break;
    case PX_KEY_NATIVE_CONTROL:
      commonKeycode = PX_KEY_CTRL;
      break;
    case PX_KEY_NATIVE_ALT:
      commonKeycode = PX_KEY_ALT;
      break;
    case PX_KEY_NATIVE_PAUSE:
      commonKeycode = PX_KEY_PAUSE;
      break;
    case PX_KEY_NATIVE_CAPSLOCK:
      commonKeycode = PX_KEY_CAPSLOCK;
      break;
    case PX_KEY_NATIVE_ESCAPE:
      commonKeycode = PX_KEY_ESCAPE;
      break;
    case PX_KEY_NATIVE_SPACE:
      commonKeycode = PX_KEY_SPACE;
      break;
    case PX_KEY_NATIVE_PAGEUP:
      commonKeycode = PX_KEY_PAGEUP;
      break;
    case PX_KEY_NATIVE_PAGEDOWN:
      commonKeycode = PX_KEY_PAGEDOWN;
      break;
    case PX_KEY_NATIVE_END:
      commonKeycode = PX_KEY_END;
      break;
    case PX_KEY_NATIVE_HOME:
      commonKeycode = PX_KEY_HOME;
      break;
    case PX_KEY_NATIVE_LEFT:
      commonKeycode = PX_KEY_LEFT;
      break;
    case PX_KEY_NATIVE_UP:
      commonKeycode = PX_KEY_UP;
      break;
    case PX_KEY_NATIVE_RIGHT:
      commonKeycode = PX_KEY_RIGHT;
      break;
    case PX_KEY_NATIVE_DOWN:
      commonKeycode = PX_KEY_DOWN;
      break;
    case PX_KEY_NATIVE_COMMA:
      commonKeycode = PX_KEY_COMMA;
      break;
    case PX_KEY_NATIVE_PERIOD:
      commonKeycode = PX_KEY_PERIOD;
      break;
    case PX_KEY_NATIVE_SLASH:
      commonKeycode = PX_KEY_FORWARDSLASH;
      break;
    case PX_KEY_NATIVE_ZERO:
      commonKeycode = PX_KEY_ZERO;
      break;
    case PX_KEY_NATIVE_ONE:
      commonKeycode = PX_KEY_ONE;
      break;
    case PX_KEY_NATIVE_TWO:
      commonKeycode = PX_KEY_TWO;
      break;
    case PX_KEY_NATIVE_THREE:
      commonKeycode = PX_KEY_THREE;
      break;
    case PX_KEY_NATIVE_FOUR:
      commonKeycode = PX_KEY_FOUR;
      break;
    case PX_KEY_NATIVE_FIVE:
      commonKeycode = PX_KEY_FIVE;
      break;
    case PX_KEY_NATIVE_SIX:
      commonKeycode = PX_KEY_SIX;
      break;
    case PX_KEY_NATIVE_SEVEN:
      commonKeycode = PX_KEY_SEVEN;
      break;
    case PX_KEY_NATIVE_EIGHT:
      commonKeycode = PX_KEY_EIGHT;
      break;
    case PX_KEY_NATIVE_NINE:
      commonKeycode = PX_KEY_NINE;
      break;
    case PX_KEY_NATIVE_SEMICOLON:
      commonKeycode = PX_KEY_SEMICOLON;
      break;
    case PX_KEY_NATIVE_EQUALS:
      commonKeycode = PX_KEY_EQUALS;
      break;
    case PX_KEY_NATIVE_A:
      commonKeycode = PX_KEY_A;
      break;
    case PX_KEY_NATIVE_B:
      commonKeycode = PX_KEY_B;
      break;
    case PX_KEY_NATIVE_C:
      commonKeycode = PX_KEY_C;
      break;
    case PX_KEY_NATIVE_D:
      commonKeycode = PX_KEY_D;
      break;
    case PX_KEY_NATIVE_E:
      commonKeycode = PX_KEY_E;
      break;
    case PX_KEY_NATIVE_F:
      commonKeycode = PX_KEY_F;
      break;
    case PX_KEY_NATIVE_G:
      commonKeycode = PX_KEY_G;
      break;
    case PX_KEY_NATIVE_H:
      commonKeycode = PX_KEY_H;
      break;
    case PX_KEY_NATIVE_I:
      commonKeycode = PX_KEY_I;
      break;
    case PX_KEY_NATIVE_J:
      commonKeycode = PX_KEY_J;
      break;
    case PX_KEY_NATIVE_K:
      commonKeycode = PX_KEY_K;
      break;
    case PX_KEY_NATIVE_L:
      commonKeycode = PX_KEY_L;
      break;
    case PX_KEY_NATIVE_M:
      commonKeycode = PX_KEY_M;
      break;
    case PX_KEY_NATIVE_N:
      commonKeycode = PX_KEY_N;
      break;
    case PX_KEY_NATIVE_O:
      commonKeycode = PX_KEY_O;
      break;
    case PX_KEY_NATIVE_P:
      commonKeycode = PX_KEY_P;
      break;
    case PX_KEY_NATIVE_Q:
      commonKeycode = PX_KEY_Q;
      break;
    case PX_KEY_NATIVE_R:
      commonKeycode = PX_KEY_R;
      break;
    case PX_KEY_NATIVE_S:
      commonKeycode = PX_KEY_S;
      break;
    case PX_KEY_NATIVE_T:
      commonKeycode = PX_KEY_T;
      break;
    case PX_KEY_NATIVE_U:
      commonKeycode = PX_KEY_U;
      break;
    case PX_KEY_NATIVE_V:
      commonKeycode = PX_KEY_V;
      break;
    case PX_KEY_NATIVE_W:
      commonKeycode = PX_KEY_W;
      break;
    case PX_KEY_NATIVE_X:
      commonKeycode = PX_KEY_X;
      break;
    case PX_KEY_NATIVE_Y:
      commonKeycode = PX_KEY_Y;
      break;
    case PX_KEY_NATIVE_Z:
      commonKeycode = PX_KEY_Z;
      break;
    case PX_KEY_NATIVE_OPENBRACKET:
      commonKeycode = PX_KEY_OPENBRACKET;
      break;
    case PX_KEY_NATIVE_BACKSLASH:
      commonKeycode = PX_KEY_BACKSLASH;
      break;
    case PX_KEY_NATIVE_CLOSEBRACKET:
      commonKeycode = PX_KEY_CLOSEBRACKET;
      break;
    case PX_KEY_NATIVE_NUMPAD0:
      commonKeycode = PX_KEY_NUMPAD0;
      break;
    case PX_KEY_NATIVE_NUMPAD1:
      commonKeycode = PX_KEY_NUMPAD1;
      break;
    case PX_KEY_NATIVE_NUMPAD2:
      commonKeycode = PX_KEY_NUMPAD2;
      break;
    case PX_KEY_NATIVE_NUMPAD3:
      commonKeycode = PX_KEY_NUMPAD3;
      break;
    case PX_KEY_NATIVE_NUMPAD4:
      commonKeycode = PX_KEY_NUMPAD4;
      break;
    case PX_KEY_NATIVE_NUMPAD5:
      commonKeycode = PX_KEY_NUMPAD5;
      break;
    case PX_KEY_NATIVE_NUMPAD6:
      commonKeycode = PX_KEY_NUMPAD6;
      break;
    case PX_KEY_NATIVE_NUMPAD7:
      commonKeycode = PX_KEY_NUMPAD7;
      break;
    case PX_KEY_NATIVE_NUMPAD8:
      commonKeycode = PX_KEY_NUMPAD8;
      break;
    case PX_KEY_NATIVE_NUMPAD9:
      commonKeycode = PX_KEY_NUMPAD9;
      break;
    case PX_KEY_NATIVE_MULTIPLY:
      commonKeycode = PX_KEY_MULTIPLY;
      break;
    case PX_KEY_NATIVE_ADD:
      commonKeycode = PX_KEY_ADD;
      break;
    case PX_KEY_NATIVE_SEPARATOR:
      commonKeycode = PX_KEY_NATIVE_SEPARATOR; //TODO
      break;
    case PX_KEY_NATIVE_SUBTRACT:
      commonKeycode = PX_KEY_SUBTRACT;
      break;
    case PX_KEY_NATIVE_DECIMAL:
      commonKeycode = PX_KEY_DECIMAL;
      break;
    case PX_KEY_NATIVE_DIVIDE:
      commonKeycode = PX_KEY_DIVIDE;
      break;
    case PX_KEY_NATIVE_F1:
      commonKeycode = PX_KEY_F1;
      break;
    case PX_KEY_NATIVE_F2:
      commonKeycode = PX_KEY_F2;
      break;
    case PX_KEY_NATIVE_F3:
      commonKeycode = PX_KEY_F3;
      break;
    case PX_KEY_NATIVE_F4:
      commonKeycode = PX_KEY_F4;
      break;
    case PX_KEY_NATIVE_F5:
      commonKeycode = PX_KEY_F5;
      break;
    case PX_KEY_NATIVE_F6:
      commonKeycode = PX_KEY_F6;
      break;
    case PX_KEY_NATIVE_F7:
      commonKeycode = PX_KEY_F7;
      break;
    case PX_KEY_NATIVE_F8:
      commonKeycode = PX_KEY_F8;
      break;
    case PX_KEY_NATIVE_F9:
      commonKeycode = PX_KEY_F9;
      break;
    case PX_KEY_NATIVE_F10:
      commonKeycode = PX_KEY_F10;
      break;
    case PX_KEY_NATIVE_F11:
      commonKeycode = PX_KEY_F11;
      break;
    case PX_KEY_NATIVE_F12:
      commonKeycode = PX_KEY_F12;
      break;
    case PX_KEY_NATIVE_DELETE:
      commonKeycode = PX_KEY_DELETE;
      break;
    case PX_KEY_NATIVE_NUMLOCK:
      commonKeycode = PX_KEY_NUMLOCK;
      break;
    case PX_KEY_NATIVE_SCROLLLOCK:
      commonKeycode = PX_KEY_SCROLLLOCK;
      break;
    case PX_KEY_NATIVE_PRINTSCREEN:
      commonKeycode = PX_KEY_PRINTSCREEN;
      break;
    case PX_KEY_NATIVE_INSERT:
      commonKeycode = PX_KEY_INSERT;
      break;

#ifdef _WIN32
	  #pragma message("WARNING: PX_KEY_NATIVE_HELP is not supported. Please fixe me!")
#else
    case PX_KEY_NATIVE_HELP:
      commonKeycode = PX_KEY_NATIVE_HELP;
      break;
#endif

    case PX_KEY_NATIVE_BACKQUOTE:
      commonKeycode = PX_KEY_GRAVEACCENT;
      break;
    case PX_KEY_NATIVE_QUOTE:
      commonKeycode = PX_KEY_SINGLEQUOTE;
      break;
    default:
      //TODO move rtLog support to pxCore so we can use here
      printf("pxWindowUtils: Unhandled keycode %d\n", nativeKeycode);
      break;
  }
  return commonKeycode;
}

