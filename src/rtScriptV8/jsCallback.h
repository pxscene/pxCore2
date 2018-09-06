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

#ifndef RT_JAVASCRIPT_CALLBACK_H
#define RT_JAVASCRIPT_CALLBACK_H


#include "headers.h"
#include <rtValue.h>
#include <vector>

namespace rtScriptV8NodeUtils
{

typedef void (*jsCallbackCompletionFunc)(void* argp, rtValue const& result);

struct jsIFunctionLookup
{
  virtual ~jsIFunctionLookup() { }
  virtual v8::Local<v8::Function> lookup(v8::Local<v8::Context>& ctx) = 0;
};

struct jsCallback
{
  virtual void enqueue();
  void registerForCompletion(jsCallbackCompletionFunc callback, void* argp);
  rtValue run();


  static jsCallback* create(v8::Local<v8::Context>& ctx);

  jsCallback* addArg(const rtValue& val);

  jsCallback* setFunctionLookup(jsIFunctionLookup* functionLookup);

  static void work(uv_work_t* req);
  static void doCallback(uv_work_t* req, int status);

  // made this public for the direct call (rtIsMain) path
  virtual ~jsCallback();

protected:
  virtual v8::Handle<v8::Value>* makeArgs(v8::Local<v8::Context>& ctx);

private:

  std::vector<rtValue> mArgs;
  uv_work_t mReq;
  jsIFunctionLookup* mFunctionLookup;

  // TODO: Is it ok to hold this pointer here?
  v8::Persistent<v8::Context> mContext;
  v8::Isolate* mIsolate;

  jsCallbackCompletionFunc mCompletionFunc;
  void* mCompletionContext;

  jsCallback(v8::Local<v8::Context>& ctx);
};

} // namespace

#endif

