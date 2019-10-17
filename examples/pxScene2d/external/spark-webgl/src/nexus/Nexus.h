//
// Created by ozgurdeveci on 3/18/17.
//
#ifndef BCMNEXUS_H
#define BCMNEXUS_H

#include <refsw/nexus_config.h>
#include <refsw/nexus_platform.h>
#include <refsw/nexus_display.h>
#include <refsw/nexus_core_utils.h>
#include <refsw/default_nexus.h>

#ifdef BCM_NEXUS_NXCLIENT
#include <refsw/nxclient.h>
#endif

#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

namespace gles2impl {
namespace BCMNexus {

    class Backend {
    private:
        Backend(const Backend&) = delete;
        Backend& operator=(const Backend&) = delete;
        Backend();

    public:
        ~Backend();

        static Backend& getInstance();

    private:
        void initialize();
        void deinitialize();

    private:
        NXPL_PlatformHandle _nxplHandle;
#ifdef BCM_NEXUS_NXCLIENT
        NxClient_AllocResults _allocResults;
#endif
    };

    class EGLTarget {
    private:
        EGLTarget() = delete;
        EGLTarget(const EGLTarget&) = delete;
        EGLTarget& operator=(const EGLTarget&) = delete;

    public:
        EGLTarget(Backend &backend, int width, int height, bool fullScreen, const std::string& title);
        ~EGLTarget() {};

        std::string constructTarget();
        void swapBuffer();
        void destroyTarget();

    private:
        void* _nativeWindow;
        Backend& _backend;
        uint32_t _width;
        uint32_t _height;
        bool _fullscreen;
        std::string _title;
        EGLDisplay  _eglDisplay;
        EGLContext  _eglContext;
        EGLSurface  _eglSurface;
    };

} // BCMNexus
} // gles2impl

#endif // BCMNEXUS_H