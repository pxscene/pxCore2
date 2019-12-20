/*

 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

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
