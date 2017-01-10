#ifndef PX_KEYBOARD_EVENT_PROVIDER_H
#define PX_KEYBOARD_EVENT_PROVIDER_H

#include <stdint.h>
#include <limits>

template<class T>
class rtPoint
{
public:
  rtPoint()
    : mX(0)
    , mY(0)
  { }

  rtPoint(T x, T y)
    : mX(x)
    , mY(y)
  { }

  inline T x() const { return mX; }
  inline T y() const { return mY; }

  inline void setX(T x) { mX = x; }
  inline void setY(T y) { mY = y; }

  static rtPoint<T> min()
  {
    return rtPoint<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::min());
  }

  static rtPoint<T> max()
  {
    return rtPoint<T>(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
  }

private:
  T mX;
  T mY;
};

template<class T>
class rtRect
{
public:
  rtRect()
    : mUpperLeft()
    , mLowerRight()
  { }

  rtRect(const rtPoint<int>& upperLeft, const rtPoint<int>& lowerRight)
    : mUpperLeft(upperLeft)
    , mLowerRight(lowerRight)
  { }

  rtRect(int x, int y, int w, int h)
    : mUpperLeft(x, y)
    , mLowerRight(x + w, y + h)
  { }

  static rtRect<T> max()
  {
    rtRect max;
    max.mUpperLeft = rtPoint<T>::min();
    max.mLowerRight = rtPoint<T>::max();
    return max;
  }

  inline rtPoint<T> upperLeft() const { return mUpperLeft; }
  inline rtPoint<T> lowerRight() const { return mLowerRight; }

private:
  rtPoint<T> mUpperLeft;
  rtPoint<T> mLowerRight;
};

enum pxKeyModifier
{
  pxKeyModifierNone   = 0,
  pxKeyModifierShift  = 0x08,
  pxKeyModifierCtrl   = 0x10,
  pxKeyModifierAlt    = 0x20,
};

typedef uint8_t pxKeyModifierSet;

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
  pxKeyModifierSet modifiers;
};

enum pxMouseEventType
{
  pxMouseEventTypeButton,
  pxMouseEventTypeMove
};

struct pxMouseEvent
{
  pxMouseEventType type;
  pxKeyModifierSet modifiers;

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
  virtual void setMousePosition(const rtPoint<int>& pos) = 0;

  // clamps mouse positioning callbacks
  virtual void setMouseBounds(const rtPoint<int>& upperLeft, const rtPoint<int>& lowerRight) = 0;

  // Allow some control of the speed of the mouse movement.
  virtual void setMouseAccelerator(int acc) = 0;

  // use this to create your platform default handler. On linux it 
  // used /dev/input/...
  static pxInputDeviceEventProvider* createDefaultProvider();
};

#endif

