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

#ifndef NODE_HEADERS_H
#define NODE_HEADERS_H

#include <rtScriptHeaders.h>

#ifdef RTSCRIPT_SUPPORT_NODE
#include <node.h>
#include <node_object_wrap.h>
#endif

#include <v8.h>
#include <v8-util.h>
#include <uv.h>

#ifdef RTSCRIPT_SUPPORT_V8

class Environment
{
  public:
    Environment(v8::Isolate* isolate, uv_loop_t* loop, v8::Platform* platform)
    {
       mIsolate = isolate;
       mUvLoop = loop;
       mPlatform = platform;
    }

    v8::Isolate* isolate()
    {
      return mIsolate;
    }

    v8::Platform* platform() {
      return mPlatform;
    }

    uv_loop_t* event_loop() {
      return mUvLoop;
    }

  private:
    v8::Isolate* mIsolate;
    v8::Platform                  *mPlatform;
    uv_loop_t                     *mUvLoop;

};

#endif

#endif

