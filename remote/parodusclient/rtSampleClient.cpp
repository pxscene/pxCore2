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
#include "rtSampleClient.h"

rtError TestObject::getCount(int& n) const { n = m_count; return RT_OK; }
rtError TestObject::setCount(int  n) { m_count = n; return RT_OK; }

void run()
{
  int n = 0;
  rtError e;
  rtValue val;

  while (true)
  {
    val = n++;

    e = obj->Set("prop", &val);
    if (e != RT_OK)
    {
      rtLogError("Set:%s", rtStrError(e));
      assert(false);
    }
    usleep(1000 * 100);

    e = obj->Get("prop", &val);
    if (e != RT_OK)
    {
      rtLogError("Get:%s - %d", rtStrError(e), val.toInt32());
      assert(false);
    }
  }
}

rtError upload_complete(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  // this matches the signature of the server's function
  rtLogError("upload_complete");
  rtLogError("argc:%d", argc);
  rtLogError("argv[0]:%s", argv[0].toString().cString());

  (void) result;
  (void) argp;

  return RT_OK;
}
