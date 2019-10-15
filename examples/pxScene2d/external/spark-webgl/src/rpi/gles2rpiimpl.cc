#include <cstring>
#include <vector>
#include <iostream>
#include <stdio.h>

#include "../gles2impl.h"

#include "bcm_host.h"

#include  <GLES2/gl2.h>
#include  <EGL/egl.h>

using namespace std;

namespace gles2impl {

static EGL_DISPMANX_WINDOW_T nativewindow;

DISPMANX_ELEMENT_HANDLE_T dispman_element;
DISPMANX_DISPLAY_HANDLE_T dispman_display;
DISPMANX_UPDATE_HANDLE_T dispman_update;

EGLDisplay  egl_display;
EGLContext  egl_context;
EGLSurface  egl_surface;

string init(int width, int height, bool fullscreen, std::string title) {
  printf("initializing DISPMANX & EGL\n");

  bcm_host_init();

  // get an EGL display connection
  egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if ( egl_display == EGL_NO_DISPLAY ) {
  	return string("Got no EGL display");
  }

  // initialize the EGL display connection
  if ( !eglInitialize( egl_display, NULL, NULL ) ) {
  	return string("Unable to initialize EGL");
  }

  static const EGLint attr[] =
  {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
  };

  // get an appropriate EGL frame buffer configuration
  EGLConfig  ecfg;
  EGLint     num_config;
  if ( !eglChooseConfig( egl_display, attr, &ecfg, 1, &num_config ) ) {
  	return string("Failed to choose config (eglError: ") + to_string(eglGetError()) + string(")");
  }

  if ( num_config != 1 ) {
  	return string("Didn't get exactly one config, but ") + to_string(num_config);
  }

  eglBindAPI(EGL_OPENGL_ES_API);

  // create an EGL rendering context
  EGLint ctxattr[] = {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
  };
  egl_context = eglCreateContext ( egl_display, ecfg, EGL_NO_CONTEXT, ctxattr );
  if ( egl_context == EGL_NO_CONTEXT ) {
  	return string("Unable to create EGL context (eglError: ") + to_string(eglGetError()) + string(")");
  }

  // Dispmanx.
  VC_RECT_T dst_rect;
  VC_RECT_T src_rect;

  uint32_t w, h;

  graphics_get_display_size(0 /* LCD */, &w, &h);

  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width = w;
  dst_rect.height = h;

  src_rect.x = 0;
  src_rect.y = 0;
  src_rect.width = width << 16;
  src_rect.height = height << 16;

  dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
  dispman_update = vc_dispmanx_update_start( 0 );

  dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
    0/*layer*/, &dst_rect, 0/*src*/,
    &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, DISPMANX_NO_ROTATE/*transform*/);

  nativewindow.element = dispman_element;
  nativewindow.width = w;
  nativewindow.height = h;
  vc_dispmanx_update_submit_sync( dispman_update );

  egl_surface = eglCreateWindowSurface ( egl_display, ecfg, &nativewindow, NULL );
  if ( egl_surface == EGL_NO_SURFACE ) {
  	return string("Unable to create EGL surface (eglError: ") + to_string(eglGetError()) + string(")");
  }

  //// associate the egl-context with the egl-surface
  if (!eglMakeCurrent( egl_display, egl_surface, egl_surface, egl_context )) {
  	return string("Unable to make EGL display the current one (eglError: ") + to_string(eglGetError()) + string(")");
  }

  return string("");
}

void nextFrame(bool swapBuffers) {
  if (swapBuffers) {
    eglSwapBuffers ( egl_display, egl_surface );  // get the rendered buffer to the screen
  }
}

void cleanup() {
  eglDestroyContext ( egl_display, egl_context );
  eglDestroySurface ( egl_display, egl_surface );
  eglTerminate      ( egl_display );
  vc_dispmanx_element_remove(dispman_update, dispman_element);
  vc_dispmanx_display_close(dispman_display);

  printf("cleanup\n");
}

} // end namespace gles2impl
