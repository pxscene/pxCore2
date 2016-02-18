// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxWindowUtil.cpp

#include <stdio.h>

#include "pxWindow.h"
#include "pxWindowUtil.h"
#include "pxCore.h"
#include "pxKeycodes.h"
#include <stdio.h>

uint32_t keycodeFromNative(uint32_t nativeKeycode)
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
// TODO
#if 0
  case PX_KEY_NATIVE_CANCEL:
    commonKeycode = PX_KEY_NATIVE_CANCEL; //TODO
    break;
#endif
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
// TODO
#if 0
  case PX_KEY_NATIVE_NUMLOCK:
    commonKeycode = PX_KEY_NUMLOCK;
    break;
#endif
  case PX_KEY_NATIVE_SCROLLLOCK:
    commonKeycode = PX_KEY_SCROLLLOCK;
    break;
  case PX_KEY_NATIVE_PRINTSCREEN:
    commonKeycode = PX_KEY_PRINTSCREEN;
    break;
  case PX_KEY_NATIVE_INSERT:
    commonKeycode = PX_KEY_INSERT;
    break;
  case PX_KEY_NATIVE_MINUS:
    commonKeycode = PX_KEY_DASH;
    break;

// TODO
#if 0
#ifdef _WIN32
#pragma message("WARNING: PX_KEY_NATIVE_HELP is not supported. Please fixe me!")
#else
  case PX_KEY_NATIVE_HELP:
    commonKeycode = PX_KEY_NATIVE_HELP;
    break;
#endif
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

struct asciiKeymapEntry
{
  uint32_t keycode;
  uint32_t flags;
  uint32_t ascii;
};

static asciiKeymapEntry asciiKeymap[] = 
{
  {PX_KEY_SPACE, 0, ' '},
  {PX_KEY_ONE, PX_MOD_SHIFT, '!'},
  {PX_KEY_SINGLEQUOTE, PX_MOD_SHIFT, '"'},
  {PX_KEY_THREE, PX_MOD_SHIFT, '#'},
  {PX_KEY_FOUR, PX_MOD_SHIFT, '$'},
  {PX_KEY_FIVE, PX_MOD_SHIFT, '%'},
  {PX_KEY_SEVEN, PX_MOD_SHIFT, '&'},
  {PX_KEY_SINGLEQUOTE, 0, '\''},
  {PX_KEY_NINE, PX_MOD_SHIFT, '('},
  {PX_KEY_ZERO, PX_MOD_SHIFT, '}'},
  {PX_KEY_EIGHT, PX_MOD_SHIFT, '*'},
  {PX_KEY_EQUALS, PX_MOD_SHIFT, '+'},
  {PX_KEY_COMMA, 0, ','},
  {PX_KEY_SUBTRACT, 0, '-'},
  {PX_KEY_PERIOD, 0, '.'},
  {PX_KEY_FORWARDSLASH, 0, '/'},
  {PX_KEY_ZERO, 0, '0'},
  {PX_KEY_ONE, 0, '1'},
  {PX_KEY_TWO, 0, '2'},
  {PX_KEY_THREE, 0, '3'},
  {PX_KEY_FOUR, 0, '4'},
  {PX_KEY_FIVE, 0, '5'},
  {PX_KEY_SIX, 0, '6'},
  {PX_KEY_SEVEN, 0, '7'},
  {PX_KEY_EIGHT, 0, '8'},
  {PX_KEY_NINE, 0, '9'},
  {PX_KEY_SEMICOLON, PX_MOD_SHIFT, ':'},
  {PX_KEY_SEMICOLON, 0, ';'},
  {PX_KEY_COMMA, PX_MOD_SHIFT, '<'},
  {PX_KEY_EQUALS, 0, '='},
  {PX_KEY_PERIOD, PX_MOD_SHIFT, '>'},
  {PX_KEY_FORWARDSLASH, PX_MOD_SHIFT, '?'},
  {PX_KEY_TWO, PX_MOD_SHIFT, '@'},
  {PX_KEY_A, PX_MOD_SHIFT, 'A'},
  {PX_KEY_B, PX_MOD_SHIFT, 'B'},
  {PX_KEY_C, PX_MOD_SHIFT, 'C'},
  {PX_KEY_D, PX_MOD_SHIFT, 'D'},
  {PX_KEY_E, PX_MOD_SHIFT, 'E'},
  {PX_KEY_F, PX_MOD_SHIFT, 'F'},
  {PX_KEY_G, PX_MOD_SHIFT, 'G'},
  {PX_KEY_H, PX_MOD_SHIFT, 'H'},
  {PX_KEY_I, PX_MOD_SHIFT, 'I'},
  {PX_KEY_J, PX_MOD_SHIFT, 'J'},
  {PX_KEY_K, PX_MOD_SHIFT, 'K'},
  {PX_KEY_L, PX_MOD_SHIFT, 'L'},
  {PX_KEY_M, PX_MOD_SHIFT, 'M'},
  {PX_KEY_N, PX_MOD_SHIFT, 'N'},
  {PX_KEY_O, PX_MOD_SHIFT, 'O'},
  {PX_KEY_P, PX_MOD_SHIFT, 'P'},
  {PX_KEY_Q, PX_MOD_SHIFT, 'Q'},
  {PX_KEY_R, PX_MOD_SHIFT, 'R'},
  {PX_KEY_S, PX_MOD_SHIFT, 'S'},
  {PX_KEY_T, PX_MOD_SHIFT, 'T'},
  {PX_KEY_U, PX_MOD_SHIFT, 'U'},
  {PX_KEY_V, PX_MOD_SHIFT, 'V'},
  {PX_KEY_W, PX_MOD_SHIFT, 'W'},
  {PX_KEY_X, PX_MOD_SHIFT, 'X'},
  {PX_KEY_Y, PX_MOD_SHIFT, 'Y'},
  {PX_KEY_Z, PX_MOD_SHIFT, 'Z'},
  {PX_KEY_OPENBRACKET, 0, '['},
  {PX_KEY_BACKSLASH, 0, '\\'},
  {PX_KEY_CLOSEBRACKET, 0, ']'},
  {PX_KEY_SIX, PX_MOD_SHIFT, '^'},
  {PX_KEY_SUBTRACT, PX_MOD_SHIFT, '_'},
  {PX_KEY_GRAVEACCENT, 0, '`'},
  {PX_KEY_A, 0, 'a'},
  {PX_KEY_B, 0, 'b'},
  {PX_KEY_C, 0, 'c'},
  {PX_KEY_D, 0, 'd'},
  {PX_KEY_E, 0, 'e'},
  {PX_KEY_F, 0, 'f'},
  {PX_KEY_G, 0, 'g'},
  {PX_KEY_H, 0, 'h'},
  {PX_KEY_I, 0, 'i'},
  {PX_KEY_J, 0, 'j'},
  {PX_KEY_K, 0, 'k'},
  {PX_KEY_L, 0, 'l'},
  {PX_KEY_M, 0, 'm'},
  {PX_KEY_N, 0, 'n'},
  {PX_KEY_O, 0, 'o'},
  {PX_KEY_P, 0, 'p'},
  {PX_KEY_Q, 0, 'q'},
  {PX_KEY_R, 0, 'r'},
  {PX_KEY_S, 0, 's'},
  {PX_KEY_T, 0, 't'},
  {PX_KEY_U, 0, 'u'},
  {PX_KEY_V, 0, 'v'},
  {PX_KEY_W, 0, 'w'},
  {PX_KEY_X, 0, 'x'},
  {PX_KEY_Y, 0, 'y'},
  {PX_KEY_Z, 0, 'z'},
  {PX_KEY_OPENBRACKET, PX_MOD_SHIFT, '{'},
  {PX_KEY_BACKSLASH, PX_MOD_SHIFT, '|'},
  {PX_KEY_CLOSEBRACKET, PX_MOD_SHIFT, '}'},
  {PX_KEY_GRAVEACCENT, PX_MOD_SHIFT, '~'},
  {PX_KEY_DASH, 0, '-'},
  {PX_KEY_DASH, PX_MOD_SHIFT, '_'},
  {PX_KEY_SUBTRACT, 0, '-'},
  {PX_KEY_SUBTRACT, PX_MOD_SHIFT, '_'}
};

static int asciiKeymapLen = sizeof(asciiKeymap)/sizeof(asciiKeymap[0]);

uint32_t keycodeToAscii(uint32_t keycode, uint32_t flags)
{
  for (int i = 0; i < asciiKeymapLen; i++)
  {
    if (asciiKeymap[i].keycode == keycode && asciiKeymap[i].flags == flags)
      return asciiKeymap[i].ascii;
  }
  return 0;
}
