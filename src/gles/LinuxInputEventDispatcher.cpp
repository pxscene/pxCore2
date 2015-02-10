#include "pxKeyboardEventProvider.h"

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

#include <string>
#include <vector>

struct RegisterdHandler
{
  pxKeyEventHandler handler;
  void* argp;
};

class LinuxInputEventDispatcher : public pxKeyboardEventProvider
{
public:
  LinuxInputEventDispatcher()
    : mFd(-1)
    , mKeyModifiers(pxKeyModifierNone)
  {
  }

  ~LinuxInputEventDispatcher()
  {
    this->close();
  }

  virtual void init()
  {
    std::string keyboardPath;

    // enumerate all available input devices. If you plug in more than
    // one keyboard, I'm going to get angry. Ideally you'd do something
    // a bit more sophisticated. For example, this won't handle 
    // hit-plugging keyboards
    DIR* d = opendir("/dev/input/by-path");
    if (!d)
    {
      printf("failed to open input device path: %d", errno);
      return;
    }

    dirent entry; // should be heap allocated
    dirent* result = NULL;

    while (true)
    {
      int ret = readdir_r(d, &entry, &result);
      if (ret > 0)
      {
        printf("failed to read input device directory: %d\n", errno);
        closedir(d);
        return;
      }

      // This is going to quit after it finds one keyboard.
      if (result && pathIsKeyboard(result->d_name))
      {
        keyboardPath = "/dev/input/by-path/";
        keyboardPath += result->d_name;
        break;
      }

      if (result == NULL)
        break;
    }

    closedir(d);

    // FYI: You can write to this device too :)
    // http://rico-studio.com/linux/read-and-write-to-a-keyboard-device/
    const char* inputDevice = keyboardPath.c_str();
    mFd = open(inputDevice, O_RDONLY);
    if (mFd == -1)
    {
      char buff[256];
      char* s = strerror_r(errno, buff, sizeof(buff));
      printf("failed to open %s. %s\n", inputDevice, s);
    }
  }

  void close()
  {
    if (mFd != -1)
      ::close(mFd);
    mFd = -1;
  }

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
        mKeyModifiers |= modifier;
      else
        mKeyModifiers &= ~modifier;
    }
    else
    {
      pxKeyEvent evt;
      evt.state = state;
      evt.modifiers = mKeyModifiers;
      evt.code = e.code;

      for (handlers_list_t::const_iterator i = mHandlers.begin(); i != mHandlers.end(); ++i)
      {
        pxKeyEventHandler handler = i->handler;
        handler(evt, i->argp);
      }
    }
  }

  virtual void addEventHandler(pxKeyEventHandler handler, void* argp)
  {
    RegisteredHandler reg;
    reg.handler = handler;
    reg.argp = argp;
    mHandlers.push_back(reg);
  }

  virtual bool next(uint32_t timeoutMillis)
  {
    mPollFds.fd = mFd;
    mPollFds.events = POLLIN | POLLERR;
    mPollFds.revents = 0;

    int ret = poll(&mPollFds, 1, timeoutMillis);

    if (ret == 0) return false;
    if (ret < 0)
    {
      printf("error processing events: %d\n", ret);
      return false;
    }

    input_event e;
    int n = read(mFd, &e, sizeof(input_event));

    switch (e.type)
    {
      case EV_KEY:
        handleKeyboardEvent(e);
        break;

        // There are a bunch.
        // https://www.kernel.org/doc/Documentation/input/event-codes.txt
      default:
        break;
    }

    return true;
  }

private:
  inline pxKeyState getKeyState(const input_event& e)
  {
    return static_cast<pxKeyState>(e.value);
  }

  pxKeyModifier getKeyModifier(const input_event& e)
  {
    const int code = e.code;
    if (code == KEY_LEFTCTRL || code == KEY_RIGHTCTRL)    return pxKeyModifierCtrl;
    if (code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT)  return pxKeyModifierShift;
    if (code == KEY_LEFTALT || code == KEY_RIGHTALT)      return pxKeyModifierAlt;
    return pxKeyModifierNone;
  }

  bool pathIsKeyboard(const char* path)
  {
    if (!path)
      return false;

    size_t n = strlen(path);
    if (n < 3)
      return false;

    return strncmp(path + n - 3, "kbd", 3) == 0;
  }

private:
  struct RegisteredHandler
  {
    pxKeyEventHandler handler;
    void* argp;
  };

  int mFd;
  pollfd mPollFds;
  uint8_t mKeyModifiers;

  typedef std::vector<RegisteredHandler> handlers_list_t;
  handlers_list_t mHandlers;
};

pxKeyboardEventProvider* pxKeyboardEventProvider::createDefaultProvider()
{
  return new LinuxInputEventDispatcher();
}

