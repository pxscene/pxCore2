

#if 1
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
//#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#endif

#include <stdio.h>
#include "rtObject.h"

class MyRtObject : public rtObject
{
public:
  virtual rtError Send(const char* message, int /*argc*/,  const rtValue* argv)
  {
    printf("MyRtObject::%s(%d)\n", message, argv[0].toInt32());
    return 0;
  }

  virtual rtError Get(const char*, rtValue*) { return 0; }
  virtual rtError Get(uint32_t, rtValue* ) { return 0; }
  virtual rtError Set(const char*, const rtValue* ) { return 0; }
  virtual rtError Set(uint32_t, const rtValue*) { return 0; }
};

void pxMain()
{
  // works as expected
  rtObject* obj = new MyRtObject();
  rtValue arg1(12345);
  obj->send("hello", 1, arg1);

  // doesn't work as expected
  rtObjectRef obj2(new MyRtObject());

  // this will crash, because MyRtObject doesn't have a 'hello' method
  // registered in rtObjectBase::Send(), but I overrode it. I think this
  // should honor the override.
  // For it to be more like std::shared_ptr<T> or other shared_ptr, it should
  // probably not inherit from rtObjectBase, but instead use syntax like:
  // obj2->send(...) where -> returns the inner pointer.
  obj2.send("hello", 1, arg1);

  // wont' compile (this is probably more correct)
  // obj2->send("hello", 1, arg1);
}

