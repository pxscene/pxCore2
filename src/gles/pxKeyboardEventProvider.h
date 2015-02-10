#ifndef PX_KEYBOARD_EVENT_PROVIDER_H
#define PX_KEYBOARD_EVENT_PROVIDER_H

#include <stdint.h>

enum pxKeyModifier
{
  pxKeyModifierNone   = 0,
  pxKeyModifierAlt    = 1,
  pxKeyModifierCtrl   = 2,
  pxKeyModifierShift  = 4
};

enum pxKeyState
{
  pxKeyStateRelease = 0,
  pxKeyStatePressed = 1,
  pxKeyStateRepeat  = 2
};

struct pxKeyEvent
{
  int code;
  pxKeyState state;

  // flags or'd together from pxKeyModifier
  uint8_t modifiers;
};

typedef void (*pxKeyEventHandler)(const pxKeyEvent& e, void* argp);

struct pxKeyboardEventProvider
{
  virtual void init() = 0;
  virtual bool next(uint32_t timeoutMillis) = 0;
  virtual void addEventHandler(pxKeyEventHandler handler, void* argp);

  static pxKeyboardEventProvider* createDefaultProvider();
};

#endif

