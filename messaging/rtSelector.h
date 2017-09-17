/* 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __RT_SELECTOR_H__
#define __RT_SELECTOR_H__

#include "rtError.h"

#include <iterator>
#include <vector>

typedef int rtSocketHandle;

class rtSelectorKey;
class rtSelector;

enum rtSelectorOps
{
  rtSelectorOps_None    = 0x00,
  rtSelectorOps_Connect = 0x01,
  rtSelectorOps_Accept  = 0x02,
  rtSelectorOps_Read    = 0x04,
  rtSelectorOps_Write   = 0x08,
  rtSelectorOps_Error   = 0x10
};

class rtSelectable
{
public:
  virtual ~rtSelectable() { }
  virtual rtSocketHandle handle() const = 0;
  virtual rtError onReadyRead(void* argp) = 0;
  virtual rtError onReadyWrite(void* argp) = 0;
  virtual rtError onReadyAccept(void* argp) = 0;
  virtual rtError onReadyConnect(void* argp) = 0;
  virtual rtError onError(rtError e, void* argp) = 0;
};

class rtSelector
{
public:
  rtError registerSelectable(rtSelectable* obj,  rtSelectorOps ops, void* argp = NULL);
  rtError select(); 

private:
  struct rtSelectorKey
  {
    rtSelectable* selectable;
    void*         arg;
    rtSelectorOps ops_set;
    rtSelectorOps ops_config;
  };

  typedef std::vector<rtSelectorKey> rtSelectorKeyList;
  rtSelectorKeyList m_keys;
  rtSelectorKeyList m_setKeys;
};

#endif
