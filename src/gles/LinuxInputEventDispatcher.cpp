#include "pxInputDeviceEventProvider.h"

#include <linux/input.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <stdint.h>
#include <dirent.h>
#include <stdlib.h>

#include <sstream>
#include <string>
#include <vector>

#include "rtLog.h"

static const char* kRootInputPath = "/dev/input/by-path/";

template <typename T>
struct Listener
{
  T callback;
  void* argp;
};

int eventCount = 0;

typedef Listener<pxKeyListener> KeyboardListener;
typedef Listener<pxMouseListener> MouseListener;

class LinuxInputEventDispatcher : public pxInputDeviceEventProvider
{
public:

  LinuxInputEventDispatcher()
    : mMouseX(0)
    , mMouseY(0)
    , mMouseAccelerator(3)
    , mMouseMoved(false)
  {
  }

  ~LinuxInputEventDispatcher()
  {
    this->close();
  }

  virtual void init()
  {
    mFds.clear();

    registerDevices(getKeyboardDevices());
    registerDevices(getMouseDevices());
  }

  virtual void setMousePosition(int x, int y)
  {
    mMouseX = x;
    mMouseY = y;
  }

  virtual void setMouseAccelerator(int acc)
  {
    mMouseAccelerator = acc;
  }

  void registerDevices(const std::vector<std::string>& devices)
  {
    for (int i = 0; i < static_cast<int>(devices.size()); ++i)
    {
      pollfd p;
      p.fd = openDevice(devices[i]);
      p.events = 0;
      p.revents = 0;
      mFds.push_back(p);
    }
  }

  void close()
  {
    for (poll_list::iterator i = mFds.begin(); i != mFds.end(); ++i)
    {
      if (i->fd != -1)
        ::close(i->fd);
      i->fd = -1;
    }

    mFds.clear();
  }

  virtual void addKeyListener(pxKeyListener listener, void* argp)
  {
    KeyboardListener eventListener;
    eventListener.callback = listener;
    eventListener.argp = argp;
    mKeyboardCallbacks.push_back(eventListener);
  }

  virtual void addMouseListener(pxMouseListener listener, void* argp)
  {
    MouseListener eventListener;
    eventListener.callback = listener;
    eventListener.argp = argp;
    mMouseCallbacks.push_back(eventListener);
  }

  virtual bool next(uint32_t timeoutMillis)
  {
    // reset event state
    for (poll_list::iterator i = mFds.begin(); i != mFds.end(); ++i)
    {
      i->events = POLLIN | POLLERR;
      i->revents = 0;
    }

    int n = poll(&mFds[0], static_cast<int>(mFds.size()), timeoutMillis);

    if (n== 0) return false;
    if (n < 0)
    {
      rtLogError("error processing events: %s", getSystemError(errno).c_str());
      return false;
    }
  
    for (poll_list::iterator i = mFds.begin(); i != mFds.end(); ++i)
    {
      pollfd& pfd = (*i);
    
      if (pfd.fd == -1) continue;
      if (!(pfd.revents & POLLIN)) continue;

      input_event e;
      int n = read(pfd.fd, &e, sizeof(input_event));

      switch (e.type)
      {
        case EV_KEY:
          switch (e.code)
          {
            case BTN_LEFT:
            case BTN_RIGHT:
            case BTN_MIDDLE:
            case BTN_SIDE:
            case BTN_EXTRA:
            handleMouseEvent(e, true);
            break;

            default:
            handleKeyboardEvent(e);
            break;
          }
          break;

        case EV_REL:
          switch (e.code)
          {
            case REL_X:
            mMouseX += (e.value * mMouseAccelerator);
            mMouseMoved = true;
            break;
            case REL_Y:
            mMouseY += (e.value * mMouseAccelerator);
            mMouseMoved = true;
            break;
          }
          break;

        case EV_SYN:
          if (mMouseMoved)
            handleMouseEvent(e, false);
          break;

          // There are a bunch.
          // https://www.kernel.org/doc/Documentation/input/event-codes.txt
        default:
          break;
      }
    }

    return true;
  }

private:
  static std::vector<std::string> getKeyboardDevices()
  {
    return getDevices(&pathIsKeyboard);
  }

  static std::vector<std::string> getMouseDevices()
  {
    return getDevices(&pathIsMouse);
  }

  static int openDevice(const std::string& path)
  {
    // FYI: You can write to this device too :)
    // http://rico-studio.com/linux/read-and-write-to-a-keyboard-device/
    const char* dev = path.c_str();
    int fd = open(dev, O_RDONLY);
    if (fd == -1)
      rtLogError("failed to open: %s: %s", dev, getSystemError(errno).c_str());
    return fd;
  }

  static std::vector<std::string> getDevices(bool (*predicate)(const char* path))
  {
    std::vector<std::string> paths;
    
    DIR* dir = opendir(kRootInputPath);
    if (!dir)
    {
      rtLogError("failed to open %s: %s", kRootInputPath, getSystemError(errno).c_str());
      return std::vector<std::string>();
    }

    dirent entry; // should be heap allocated @see map opendir_r for details
    dirent* result = NULL;
   
    do
    {
      int ret = readdir_r(dir, &entry, &result);
      if (ret > 0)
      {
        rtLogError("failed reading %s: %s", kRootInputPath, getSystemError(errno).c_str());
        closedir(dir);
        return paths;
      }

      if (result && predicate(result->d_name))
      {
        std::string path = kRootInputPath;
        path += result->d_name;
        paths.push_back(path);
      }
    }
    while (result != NULL);

    return paths;
  }

  static std::string getSystemError(int err)
  {
    char buff[256];
    const char* s = strerror_r(err, buff, sizeof(buff));
    if (s)
      return std::string(s);

    std::stringstream out;
    out << "unknown error:" << err;
    return out.str();
  }


  inline pxKeyState getKeyState(const input_event& e)
  {
    return static_cast<pxKeyState>(e.value);
  }

  static pxKeyModifier getKeyModifier(const input_event& e)
  {
    const int code = e.code;
    if (code == KEY_LEFTCTRL || code == KEY_RIGHTCTRL)    return pxKeyModifierCtrl;
    if (code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT)  return pxKeyModifierShift;
    if (code == KEY_LEFTALT || code == KEY_RIGHTALT)      return pxKeyModifierAlt;
    return pxKeyModifierNone;
  }

  static bool pathIsMouse(const char* devname)
  {
    if (!devname)
      return false;
    
    size_t n = strlen(devname);
    if (n < 11)
      return false;

    return strncmp(devname + n - 11, "event-mouse", 11) == 0;
  }

  static bool pathIsKeyboard(const char* path)
  {
    if (!path)
      return false;

    size_t n = strlen(path);
    if (n < 3)
      return false;

    return strncmp(path + n - 3, "kbd", 3) == 0;
  }

  static pxMouseButton getMouseButton(const input_event& e)
  {
    switch (e.code)
    {
      case BTN_LEFT:    return pxMouseButtonLeft; break;
      case BTN_RIGHT:   return pxMouseButtonRight; break;
      case BTN_MIDDLE:  return pxMouseButtonMiddle; break;
      case BTN_SIDE:    return pxMouseButtonSide; break;
      case BTN_EXTRA:   return pxMouseButtonExtra; break;
      default:
        assert(false);
        break;
    }
    return pxMouseButtonMiddle;
  }

private:
  typedef std::vector<pollfd> poll_list;

  void handleKeyboardEvent(const input_event& e)
  {
    assert(e.type == EV_KEY);
    pxKeyState state = getKeyState(e);
    pxKeyModifier modifier = getKeyModifier(e);

    // for specific key modifiers we don't pass them off to the user, 
    // we simpy record the current state in the mKeyModifers bit field.
    if (modifier != pxKeyModifierNone)
    {
      if (state == pxKeyStatePressed || state == pxKeyStateRepeat)
        mModifiers |= modifier;
      else
        mModifiers &= ~modifier;
    }
    else
    {
      pxKeyEvent evt;
      evt.state = state;
      evt.modifiers = mModifiers;
      evt.code = e.code;

      // rtLogInfo("keyevent {state:%d modifiers:%d code:%d}", evt.state, evt.modifiers, evt.code);
      for (keyboard_listeners::const_iterator i = mKeyboardCallbacks.begin(); i !=
          mKeyboardCallbacks.end(); ++i)
      {
        pxKeyListener callback = i->callback;
        callback(evt, i->argp);
      }
    }
  }

  void handleMouseEvent(const input_event& e, bool isButtonPress)
  {
    pxMouseEvent evt;
    if (isButtonPress)
    {
      evt.type = pxMouseEventTypeButton;
      evt.button.state = getKeyState(e);
      evt.button.button = getMouseButton(e);
    }
    else
    {
      evt.type = pxMouseEventTypeMove;
      evt.move.x = mMouseX;
      evt.move.y = mMouseY;
    }

    // rtLogInfo("mouse button {code:%d value:%d}", e.code, e.value);

    for (mouse_listeners::const_iterator i = mMouseCallbacks.begin(); i !=
      mMouseCallbacks.end(); ++i)
    {
      pxMouseListener callback = i->callback;
      callback(evt, i->argp);
    }

    mMouseMoved = false;
  }

private:
  poll_list mFds;
  uint8_t mModifiers;

  typedef std::vector<KeyboardListener> keyboard_listeners;
  keyboard_listeners mKeyboardCallbacks;

  typedef std::vector<MouseListener> mouse_listeners;
  mouse_listeners mMouseCallbacks;

  int mMouseX;
  int mMouseY;
  int mMouseAccelerator;
  bool mMouseMoved;
};

pxInputDeviceEventProvider* pxInputDeviceEventProvider::createDefaultProvider()
{
  return new LinuxInputEventDispatcher();
}

#if 0
void mouseHandler(const pxMouseEvent& e, void* argp)
{
  if (e.type == pxMouseEventTypeButton)
  {
    printf("press: {state:%d button:%d}\n", e.button.state, e.button.button);
  }
  else if (e.type == pxMouseEventTypeMove)
  {
    printf("move : {x:%d y:%d}\n", e.move.x, e.move.y);
  }
  else
  {
    printf("unknown mouse event");
  }
}

void keyHandler(const pxKeyEvent& e, void* argp)
{
  printf("key {code:%d state:%d modifiers:%d}\n", e.code, e.state, e.modifiers);
}

int main(int argc, char* argv[])
{
  pxInputDeviceEventProvider* p = pxInputDeviceEventProvider::createDefaultProvider();  
  p->init();
  p->addMouseListener(mouseHandler, NULL);
  p->addKeyListener(keyHandler, NULL);
  p->setMousePosition(640, 360); // .5 1280x720 (middle)
  while (true)
  {
    p->next(1000);
  }
  return 0;
}
#endif
