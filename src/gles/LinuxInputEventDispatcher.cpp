#include "pxInputDeviceEventProvider.h"

#include <linux/input.h>
#include <sys/inotify.h>
#include <sys/stat.h>
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
#include <stddef.h>

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

// TODO figure out what to do with rtLog
#if 0
#include "rtLog.h"
#else
#define rtLogWarn printf
#define rtLogError printf
#define rtLogFatal printf
#define rtLogInfo printf
#define rtLogDebug printf
#endif
static const char* kDevInputByPath = "/dev/input/by-path/";
static const char* kDevInput       = "/dev/input";
static const int   kMaxOpendirTries = 3;

template <typename T>
struct Listener
{
  T callback;
  void* argp;
};

typedef Listener<pxKeyListener> KeyboardListener;
typedef Listener<pxMouseListener> MouseListener;

static bool deviceExists(const char* name)
{
  char path[256];
  strcpy(path, kDevInput);
  strcat(path, "/");
  strcat(path, name);

  struct stat buf;
  return stat(path, &buf) == 0;
}

static bool waitForDevice(const char* devname)
{
  rtLogDebug("waiting for %s to be created", devname);

  bool wait = false;
  for (int i = 0; i < 5; ++i)
  {
    if (wait)
      usleep(1000 * 200);

    struct stat buf;
    if (::stat(kDevInputByPath, &buf) == 0)
    {
      rtLogDebug("found: %s", devname);
      return true;
    }

    rtLogDebug("%s doesn't exist, waiting", devname);
    wait = true;
  }

  return false;
}

struct NotDescriptor
{
  NotDescriptor(int i) 
    : mFd(i) { }

  bool operator()(const pollfd& p)
  {
    return p.fd != mFd;
  }
private:
  int mFd;
};

class LinuxInputEventDispatcher : public pxInputDeviceEventProvider
{
  typedef std::vector<pollfd> poll_list;

public:

  LinuxInputEventDispatcher()
    : mModifiers(0)
    , mMouseX(0)
    , mMouseY(0)
    , mMouseAccelerator(3)
    , mMouseMoved(false)
    , mInotifyFd(-1)
    , mWatchFd(-1)
  {
    long nameMax = pathconf(kDevInputByPath, _PC_NAME_MAX);
    if (nameMax == -1)
      nameMax = 255;

    long buffSize = offsetof(struct dirent, d_name) + nameMax + 1;
    mDirEntryBuffer.reserve(buffSize);
    mDirEntryBuffer.resize(buffSize);
    mNotifyBuffer.reserve(512);
    mNotifyBuffer.resize(512);
  }

  ~LinuxInputEventDispatcher()
  {
    this->close(true);
  }

  virtual void stop()
  {
    rtLogWarn("implement me");
  }

  virtual void init()
  {
    if (mInotifyFd == -1)
    {
      mInotifyFd = inotify_init1(IN_CLOEXEC);
      if (mInotifyFd == -1)
      {
        rtLogWarn("hotplug disabled: %s", getSystemError(errno).c_str());
      }
      else
      {
        rtLogDebug("initializing inotify, adding watch: %s", kDevInput);
        mWatchFd = inotify_add_watch(mInotifyFd, kDevInput, (IN_DELETE | IN_CREATE));
        if (mWatchFd == -1)
        {
          rtLogWarn("hotplug disabled: %s", getSystemError(errno).c_str());
        }
        else
        {
          rtLogDebug("adding change notify descriptor: %d to poll list", mInotifyFd);
          pollfd p;
          p.fd = mInotifyFd;
          p.events = (POLLIN | POLLERR);
          p.revents = 0;
          mFds.push_back(p);
        }
      }
    }

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
    typedef std::vector<std::string>::const_iterator iterator;
    for (iterator i = devices.begin(); i != devices.end(); ++i)
    {
      pollfd p;
      p.fd = openDevice(*i);
      p.events = 0;
      p.revents = 0;
      mFds.push_back(p);
    }
  }

  void close(bool closeWatch)
  {
    {
      // close everything but the watch notify
      poll_list::iterator i = std::remove_if(mFds.begin(), mFds.end(), NotDescriptor(mInotifyFd));
      closeAll(i, mFds.end());
      mFds.erase(i, mFds.end());
    }

    // closeWatch == false means leave the inotify open
    if (closeWatch)
    {
      if (mWatchFd != -1)
        inotify_rm_watch(mInotifyFd, mWatchFd);

      // there should really only be one left in here
      assert(mFds.size() == 1);

      closeAll(mFds.begin(), mFds.end());
      mFds.clear();

      mWatchFd = -1;
      mInotifyFd = -1;
    }
  }

  void closeAll(poll_list::iterator begin, poll_list::iterator end)
  {
    while (begin != end)
    {
      safeClose(begin->fd);
      ++begin;
    }
  }

  static void safeClose(int fd)
  {
    rtLogDebug("close: %d", fd);
    if (fd != -1)
      ::close(fd);
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
    if (n < 0)
    {
      int err = errno;
      if (err == EINTR)
      {
        rtLogInfo("poll was interrupted by EINTR?");
      }
      else
      {
        rtLogError("error processing events: %s", getSystemError(err).c_str());
        return false;
      }
    }

    bool deviceListChanged = false;
    for (poll_list::iterator i = mFds.begin(); i != mFds.end(); ++i)
    {
      if (i->fd == -1)
      {
        rtLogWarn("invalid file descriptor");
        continue;
      }

      if (!(i->revents & POLLIN))
        continue;

      if (i->fd == mInotifyFd)
        deviceListChanged = processChangeNotify(*i);
      else
        processDescriptor(*i);
    }

    if (deviceListChanged)
    {
      rtLogInfo("device list change notify");
      close(false);
      init();
    }
    return true;
  }

  bool processChangeNotify(const pollfd& pfd)
  {
    assert(pfd.fd != -1);
    assert(pfd.revents & POLLIN);

    bool importantChange = false;
    int n = read(pfd.fd, &mNotifyBuffer[0], static_cast<int>(mNotifyBuffer.size()));
    if (n == -1)
    {
      rtLogWarn("failed to read change notify buffer: %d", n);
      return false;
    }
    else
    {
      inotify_event* e = reinterpret_cast<inotify_event *>(&mNotifyBuffer[0]);
      if (e->len)
      {
        size_t n = strlen(e->name);
        if ((n > 4) && (strncmp(e->name, "event", 5) == 0))
        {
          importantChange = true;
          if (deviceExists(e->name))
          {
            rtLogInfo("%s added", e->name);
            waitForDevice(kDevInputByPath);
          }
          else
          {
            rtLogInfo("device removed: %s", e->name);
          }
        }
      }
    }
    return importantChange;
  }

  void processDescriptor(const pollfd& pfd)
  {
    assert(pfd.fd != -1);
    assert(pfd.revents & POLLIN);

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

private:
  std::vector<std::string> getKeyboardDevices()
  {
    return getDevices(&pathIsKeyboard);
  }

  std::vector<std::string> getMouseDevices()
  {
    return getDevices(&pathIsMouse);
  }

  static int openDevice(const std::string& path)
  {
    // FYI: You can write to this device too :)
    // http://rico-studio.com/linux/read-and-write-to-a-keyboard-device/
    const char* dev = path.c_str();
    int fd = open(dev, O_RDONLY | O_CLOEXEC);
    if (fd == -1)
      rtLogError("failed to open: %s: %s", dev, getSystemError(errno).c_str());
    else
      rtLogDebug("opened: %d for: %s", fd, path.c_str());
    return fd;
  }

  std::vector<std::string> getDevices(bool (*predicate)(const char* path))
  {
    std::vector<std::string> paths;

    DIR* dir = opendir(kDevInputByPath);
    if (!dir)
    {
// TODO figure out what to do with rtLog
#if 0
      int err = errno;
      rtLogLevel level = RT_LOG_DEBUG;
      if (err != ENOENT)
        level = RT_LOG_WARN;
      rtLog(level, "failed to open %s: %s", kDevInputByPath, getSystemError(err).c_str());
#endif
      return std::vector<std::string>();
    }

    dirent* entry = reinterpret_cast<dirent *>(&mDirEntryBuffer[0]);
    dirent* result = NULL;

    do
    {
      int ret = readdir_r(dir, entry, &result);
      if (ret > 0)
      {
        rtLogError("failed reading %s: %s", kDevInputByPath, getSystemError(errno).c_str());
        closedir(dir);
        return paths;
      }

      if (result && predicate(result->d_name))
      {
        std::string path = kDevInputByPath;
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

  static bool pathIsKeyboard(const char* devname)
  {
    if (!devname)
      return false;

    size_t n = strlen(devname);
    if (n < 3)
      return false;

    return strncmp(devname+ n - 3, "kbd", 3) == 0;
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

      // rtLogDebug("keyevent {state:%d modifiers:%d code:%d}", evt.state, evt.modifiers, evt.code);
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
      evt.modifiers = mModifiers;
      evt.button.state = getKeyState(e);
      evt.button.button = getMouseButton(e);
      evt.button.x = mMouseX;
      evt.button.y = mMouseY;
    }
    else
    {
      evt.type = pxMouseEventTypeMove;
      evt.move.x = mMouseX;
      evt.move.y = mMouseY;
    }

    //     rtLogDebug("mouse button {code:%d value:%d}", e.code, e.value);
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

  std::vector<uint8_t> mDirEntryBuffer;
  std::vector<uint8_t> mNotifyBuffer;

  int mInotifyFd;
  int mWatchFd;
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
