#ifndef PX_KEYBOARD_EVENT_PROVIDER_H
#define PX_KEYBOARD_EVENT_PROVIDER_H

#include <stdint.h>

enum pxKeyModifier
{
  pxKeyModifierNone   = 0,
  pxKeyModifierShift  = 0x08,
  pxKeyModifierCtrl   = 0x10,
  pxKeyModifierAlt    = 0x20,
};

enum pxKeyState
{
  pxKeyStateRelease = 0,
  pxKeyStatePressed = 1,
  pxKeyStateRepeat  = 2
};

enum pxMouseButton
{
  pxMouseButtonNone,
  pxMouseButtonLeft,
  pxMouseButtonRight,
  pxMouseButtonMiddle,
  pxMouseButtonSide,
  pxMouseButtonExtra
};

struct pxKeyEvent
{
  int code;
  pxKeyState state;

  // flags or'd together from pxKeyModifier
  uint8_t modifiers;
};

enum pxMouseEventType
{
  pxMouseEventTypeButton,
  pxMouseEventTypeMove
};

struct pxMouseEvent
{
  pxMouseEventType type;

  union 
  {
    struct
    {
      int x;
      int y;
    } move;

    struct
    {
      pxMouseButton button;
      pxKeyState state;
      int x;
      int y;
    } button;
  };
};

typedef void (*pxKeyListener)(const pxKeyEvent& e, void* argp);
typedef void (*pxMouseListener)(const pxMouseEvent& e, void* argp);

struct pxInputDeviceEventProvider
{
  // call this first after instantiation
  virtual void init() = 0;

  // This should break any thread from a call to next()
  virtual void stop() = 0;

  // call while (true) on this in a dedicated thread. The callbacks
  // get dispatched in the context of the calling thread.
  virtual bool next(uint32_t timeoutMillis) = 0;
  virtual void addKeyListener(pxKeyListener, void* argp) = 0;
  virtual void addMouseListener(pxMouseListener listener, void* argp) = 0;

  // Allow the client to control the starting point of the mouse.
  virtual void setMousePosition(int x, int y) = 0;

  // Allow some control of the speed of the mouse movement.
  virtual void setMouseAccelerator(int acc) = 0;

  // use this to create your platform default handler. On linux it 
  // used /dev/input/...
  static pxInputDeviceEventProvider* createDefaultProvider();
};

#endif

