#include <cstring>
#include <vector>
#include <iostream>
#include <cstdlib>

#include "../gles2impl.h"

#include "Nexus.h"

using namespace std;

namespace gles2impl {

    BCMNexus::EGLTarget *eglTarget;

    string init(int width, int height, bool fullscreen, std::string title) {
        cout << "initializing BCM NEXUS & EGL" << endl;

        eglTarget = new BCMNexus::EGLTarget(BCMNexus::Backend::getInstance(), width, height, fullscreen, title);
        std::string result = eglTarget->constructTarget();

        // This comment is taken from webkit
        // EGL registers atexit handlers to cleanup its global display list.
        // Since the global EGLTarget instance is created before,
        // when the EGLTarget destructor is called, EGL has already removed the
        // display from the list, causing eglTerminate() to crash. So, here we register
        // our own atexit handler, after EGL has been initialized and after the global
        // instance has been created to ensure we call eglTerminate() before the other
        // EGL atexit handlers and the EGLTarget destructor.
        std::atexit(cleanup);

        return result;
    }

    void nextFrame(bool swapBuffers) {
        if ((eglTarget != nullptr) && swapBuffers == true) {
            eglTarget->swapBuffer();
        }
    }

    void cleanup() {
        cout << "Destroy EGL target" << endl;

        if (eglTarget != nullptr) {
            eglTarget->destroyTarget();
            delete eglTarget;
            eglTarget = nullptr;
        }
    }

} // end namespace gles2impl
