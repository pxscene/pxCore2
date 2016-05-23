#ifndef RT_JAVA_SCRIPT_CALLBACK_H
#define RT_JAVA_SCRIPT_CALLBACK_H

#include "node_headers.h"
#include <rtValue.h>
#include <vector>

using namespace v8;

typedef void (*jsCallbackCompletionFunc)(void* argp, rtValue const& result);

struct jsIFunctionLookup
{
  virtual ~jsIFunctionLookup() { }
  virtual Local<Function> lookup() = 0;
};

struct jsCallback
{
  virtual Handle<Value>* makeArgs();
  virtual void enqueue();
  void registerForCompletion(jsCallbackCompletionFunc callback, void* argp);
  rtValue run();


  static jsCallback* create(v8::Isolate* isolate);

  jsCallback* addArg(const rtValue& val);

  jsCallback* setFunctionLookup(jsIFunctionLookup* functionLookup);

  static void work(uv_work_t* req);
  static void doCallback(uv_work_t* req, int status);

  // made this public for the direct call (rtIsMain) path
  virtual ~jsCallback();

private:
  std::vector<rtValue> mArgs;
  uv_work_t mReq;
  jsIFunctionLookup* mFunctionLookup;

  // TODO: Is it ok to hold this pointer here?
  v8::Isolate* mIsolate;
  jsCallbackCompletionFunc mCompletionFunc;
  void* mCompletionContext;

  jsCallback(v8::Isolate* isolate);
};

#endif

