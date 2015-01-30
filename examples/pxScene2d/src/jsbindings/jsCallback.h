#ifndef RT_JAVA_SCRIPT_CALLBACK_H
#define RT_JAVA_SCRIPT_CALLBACK_H

#include <node.h>
#include <v8.h>

#include <rtValue.h>
#include <vector>

using namespace v8;

struct jsIFunctionLookup
{
  virtual ~jsIFunctionLookup() { }
  virtual Persistent<Function> lookup() = 0;
};

struct jsCallback
{
  virtual Handle<Value>* makeArgs();
  virtual void enqueue();

  static jsCallback* create();

  jsCallback* addArg(const rtValue& val);

  jsCallback* setFunctionLookup(jsIFunctionLookup* functionLookup);

  static void work(uv_work_t* req);
  static void doCallback(uv_work_t* req, int status);

private:
  std::vector<rtValue> mArgs;
  uv_work_t mReq;
  jsIFunctionLookup* mFunctionLookup;

  jsCallback(); 
  virtual ~jsCallback();
};

#endif

