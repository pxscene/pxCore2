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
  // call this first after instantiation
  virtual void init() = 0;

  // call while (true) on this in a dedicated thread. The callbacks
  // get dispatched in the context of the calling thread.
  virtual bool next(uint32_t timeoutMillis) = 0;
  virtual void addEventHandler(pxKeyEventHandler handler, void* argp) = 0;

  // use this to create your platform default handler. On linux it 
  // used /dev/input/...
  static pxKeyboardEventProvider* createDefaultProvider();
};

#endif

