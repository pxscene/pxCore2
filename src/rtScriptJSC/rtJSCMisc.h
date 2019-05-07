#ifndef RTJSCMISC_H
#define RTJSCMISC_H

#include <JavaScriptCore/JavaScript.h>
#include "rtString.h"
#include "rtAtomic.h"

#include <string>
#include <functional>
#include <vector>

namespace RtJSC {

template<typename T>
class RefCounted: public T
{
protected:
  int m_refCount { 0 };
  unsigned long AddRef() final {
    return rtAtomicInc(&m_refCount);
  }
  unsigned long Release() final {
    long l = rtAtomicDec(&m_refCount);
    if (l == 0) {
      delete this;
    }
    return l;
  }
  virtual ~RefCounted() {}
};

void initMainLoop();
void pumpMainLoop();
void dispatchOnMainLoop(std::function<void ()>&& fun);
uint32_t installTimeout(double intervalMs, bool repeat, std::function<int ()>&& fun);
void clearTimeout(uint32_t tag);
void printException(JSContextRef ctx, JSValueRef exception);
rtString jsToRtString(JSStringRef str);
bool fileExists(const char* name);
std::string readFile(const char *file);
std::vector<uint8_t> readBinFile(const char *file);

void assertIsMainThread();

}

#endif /* RTJSCMISC_H */
