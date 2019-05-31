//
// Created by ozgurdeveci on 3/18/17.
//
#include <cstring>
#include <iostream>
#include <cassert>

#include "Nexus.h"

namespace gles2impl {
namespace BCMNexus {

    Backend::Backend() {

        initialize();
    }

    Backend::~Backend() {

        deinitialize();
    }

/*static */ Backend& Backend::getInstance() {

        static Backend instance;

        return instance;
    }

    void Backend::initialize() {

        NEXUS_DisplayHandle displayHandle(nullptr);
#ifdef BCM_NEXUS_NXCLIENT
        NxClient_AllocSettings allocSettings;
        NxClient_JoinSettings joinSettings;
        NxClient_GetDefaultJoinSettings(&joinSettings);

        strcpy(joinSettings.name, "node-nexus-backend");

        NEXUS_Error rc = NxClient_Join(&joinSettings);
        BDBG_ASSERT(!rc);

        NxClient_GetDefaultAllocSettings(&allocSettings);
        allocSettings.surfaceClient = 1;
        rc = NxClient_Alloc(&allocSettings, &_allocResults);
        BDBG_ASSERT(!rc);
#else
        NEXUS_Error rc = NEXUS_Platform_Join();
        BDBG_ASSERT(!rc);
#endif
        NXPL_RegisterNexusDisplayPlatform(&_nxplHandle, displayHandle);
    }

    void Backend::deinitialize() {

        NXPL_UnregisterNexusDisplayPlatform(_nxplHandle);
#ifdef BCM_NEXUS_NXCLIENT
        NxClient_Free(&_allocResults);
        NxClient_Uninit();
#endif
    }

    EGLTarget::EGLTarget(Backend &backend, int width, int height, bool fullScreen, const std::string& title)
        : _backend(backend)
        , _width(width)
        , _height(height)
        , _fullscreen(fullScreen)
        , _title(title) {
    }

    std::string EGLTarget::constructTarget() {

        // Get native window
        NXPL_NativeWindowInfo windowInfo;
        windowInfo.x = 0;
        windowInfo.y = 0;
        windowInfo.width = _width;
        windowInfo.height = _height;
        windowInfo.stretch = _fullscreen;
        windowInfo.zOrder = 0;
        windowInfo.clientID = 0; //TODO: do not use hardcoded
        _nativeWindow = NXPL_CreateNativeWindow(&windowInfo);

        //   Get the EGL display.
        _eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (_eglDisplay == EGL_NO_DISPLAY) {
            std::cout << "eglGetDisplay() failed, did you register any exclusive displays" << std::endl;
            return std::string("Got no EGL display");
        }

        //   Initialize EGL.
        //   EGL has to be initialized with the display obtained in the
        //   previous step. We cannot use other EGL functions except
        //   eglGetDisplay and eglGetError before eglInitialize has been
        //   called.
        if (!eglInitialize(_eglDisplay, nullptr, nullptr)) {
            std::cout << "eglInitialize() failed" << std::endl;
            return std::string("Unable to initialize EGL");
        }
        //   Step 3 - Get the number of configurations to correctly size the array
        //   used in step 4
        static const EGLint attr[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
        };

        EGLConfig eglConfig;
        EGLint     num_config;

        if ( !eglChooseConfig( _eglDisplay, attr, &eglConfig, 1, &num_config ) ) {
            return std::string("Failed to choose config (eglError: ") + std::to_string(eglGetError()) + std::string(")");
        }

        if ( num_config != 1 ) {
            return std::string("Didn't get exactly one config, but ") + std::to_string(num_config);
        }

        // create an EGL rendering context
        EGLint ctxattr[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        _eglContext = eglCreateContext ( _eglDisplay, eglConfig, EGL_NO_CONTEXT, ctxattr );
        if ( _eglContext == EGL_NO_CONTEXT ) {
            return std::string("Unable to create EGL context (eglError: ") + std::to_string(eglGetError()) + std::string(")");
        }

        /*
        Step 5 - Create a surface to draw to.
        Use the config picked in the previous step and the native window
        handle to create a window surface.
        */
        _eglSurface = eglCreateWindowSurface(_eglDisplay, eglConfig, _nativeWindow, NULL);
        if (_eglSurface == EGL_NO_SURFACE)
        {
            std::cout << "eglCreateWindowSurface() failed" << std::endl;
            return std::string("Unable to create EGL surface (eglError: ") + std::to_string(eglGetError()) + std::string(")");
        }

        eglBindAPI(EGL_OPENGL_ES_API);

        //// associate the egl-context with the egl-surface
        if (!eglMakeCurrent( _eglDisplay, _eglSurface, _eglSurface, _eglContext)) {
            return std::string("Unable to make EGL display the current one (eglError: ") + std::to_string(eglGetError()) + std::string(")");
        }

        return std::string("");
    }

    void EGLTarget::destroyTarget() {

        if (_eglSurface != EGL_NO_SURFACE) {
            assert(_eglDisplay != EGL_NO_DISPLAY);
            eglDestroySurface(this->_eglDisplay, this->_eglSurface);
            _eglSurface = EGL_NO_SURFACE;
        }
        if (_eglContext != EGL_NO_CONTEXT) {
            assert(_eglDisplay != EGL_NO_DISPLAY);
            eglDestroyContext(_eglDisplay, _eglContext);
            _eglContext = EGL_NO_CONTEXT;
        }
        if (_eglDisplay != EGL_NO_DISPLAY) {
            eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglTerminate(_eglDisplay);
            _eglDisplay = EGL_NO_DISPLAY;
        }

        if (_nativeWindow != nullptr) {
            NXPL_DestroyNativeWindow(_nativeWindow);
            _nativeWindow = nullptr;
        }

        std::cout << "cleanup" << std::endl;
    }

    void EGLTarget::swapBuffer() {

        eglSwapBuffers (_eglDisplay, _eglSurface);
    }

} // BCMNexus
} // gles2impl