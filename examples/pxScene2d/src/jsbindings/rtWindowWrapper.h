#ifndef RT_WINDOW_WRAPPER_H
#define RT_WINDOW_WRAPPER_H

#include <node.h>
#include <v8.h>

using namespace v8;

class jsWindow;
class pxWindow;

class rtWindowWrapper : public node::ObjectWrap
{
public:
  static void exportPrototype(Handle<Object> exports);

private:
  static Handle<Value> create(const Arguments& args);
  static Handle<Value> close(const Arguments& args);
  static Handle<Value> on(const Arguments& args);

  static void setVisible(Local<String> prop, Local<Value> value, const AccessorInfo& info);
  static Handle<Value> getVisible(Local<String> prop, const AccessorInfo& info);
  static Handle<Value> getScene(Local<String> prop, const AccessorInfo& info);

  static void setTitle(Local<String> prop, Local<Value> value, const AccessorInfo& info);

  static inline pxWindow* unwrap(const AccessorInfo& info) 
  {
    return node::ObjectWrap::Unwrap<rtWindowWrapper>(info.This())->mWindow;
  }
    
  static inline pxWindow* unwrap(const Arguments& args)
  {
    return node::ObjectWrap::Unwrap<rtWindowWrapper>(args.This())->mWindow;
  }

private:
  pxWindow* mWindow;
};

#endif

