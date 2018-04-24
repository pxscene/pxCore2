/*
 * Copyright [2017] [Comcast, Corp.]
 *
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
#include "rtRemote.h"
#include <thread>
#include <unistd.h>
#include <assert.h>


class TestObject : public rtObject
{
  rtDeclareObject(TestObject, rtObject);
public:
  rtProperty(count, getCount, setCount, int);
  rtError getCount(int& n) const;
  rtError setCount(int  n); 
private:
  int m_count;
    
};

//rtObjectRef obj;
void run();
rtError upload_complete(int argc, rtValue const* argv, rtValue* result, void* argp);
