#ifndef WINDOWS_GL_CONTEXT
#define WINDOWS_GL_CONTEXT
#include "pxContext.h"

class WindowsGLContext
{
public:
  static WindowsGLContext* instance() {
    if (mContext == nullptr) {
      mContext = new WindowsGLContext();
      mContext->init();
    }
    return mContext;
  }

  void init() {
  }

  HGLRC createContext() {
    return ::wglCreateContext(hdc);
  }

  void makeCurrent(HGLRC hrc) {
    if (::wglMakeCurrent(hdc, hrc)) {
      glewExperimental = GL_TRUE;
      if (glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

      char *GL_version = (char *)glGetString(GL_VERSION);
      char *GL_vendor = (char *)glGetString(GL_VENDOR);
      char *GL_renderer = (char *)glGetString(GL_RENDERER);


      rtLogInfo("GL_version = %s", GL_version);
      rtLogInfo("GL_vendor = %s", GL_vendor);
      rtLogInfo("GL_renderer = %s", GL_renderer);
    }
    else {
      throw std::runtime_error("gl context makeCurrent failed !!");
    }
  }

  void deleteContext(HGLRC hrc) {
    ::wglDeleteContext(hrc);
  }

  ~WindowsGLContext() {

  }

  HDC hdc;
  HGLRC rootContext;
  static WindowsGLContext* mContext;
private:
  WindowsGLContext() {}
};
#endif // !WINDOWS_GL_CONTEXT
