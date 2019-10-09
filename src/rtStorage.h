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

// rtStorage.h

#ifndef _RT_STORAGE
#define _RT_STORAGE

#include "rtCore.h"
#include "rtObject.h"

class rtStorage: public rtObject
{
public:
  rtStorage(const char* filename, const uint32_t storageQuota = 0, const char* key = NULL);
  virtual ~rtStorage();

  rtDeclareObject(rtStorage, rtObject);
  rtMethod2ArgAndNoReturn("setItem",setItem,rtString,rtValue);
  rtMethod1ArgAndReturn("getItem",getItem,rtString,rtValue);

  rtMethod1ArgAndReturn("getItems",getItems,rtString,rtObjectRef);

  rtMethod1ArgAndNoReturn("removeItem",removeItem,rtString);
  rtMethodNoArgAndNoReturn("clear",clear);

  rtError init(const char* filename, uint32_t storageQuota = 0, const char* key = NULL);

  // closes file
  rtError term();

  rtError setItem(const char* key, const rtValue& value);
  rtError getItem(const char* key, rtValue& retValue) const;

  // Return an object for each key/value pair where the key starts with keyPrefix
  // each object returned will have a 'key" property and a 'value' property
  rtError getItems(const char* keyPrefix, rtObjectRef& retValue) const;

  rtError removeItem(const char* key);

  // Clear all data
  rtError clear();

  rtError runVacuumCommand();

  static bool isEncrypted(const char* fileName);

  virtual rtError Get(const char* name, rtValue* value) const;
  virtual rtError Get(uint32_t /*i*/, rtValue* /*value*/) const;
  virtual rtError Set(const char* name, const rtValue* value);
  virtual rtError Set(uint32_t /*i*/, const rtValue* /*value*/);

private:
  void* mPrivateData;
  uint32_t mQuota;

  // Methods for managing current size and quota
  rtError calculateCurrentSize(uint32_t &size) const;
  rtError setCurrentSize(const uint32_t size);
  rtError getCurrentSize(const char* key, uint32_t& sizeForKey, uint32_t& sizeTotal) const;

  rtError updateSize();
  rtError verifyQuota(const char* key, const rtValue& value) const;
};

typedef rtRef<rtStorage> rtStorageRef;

#endif
