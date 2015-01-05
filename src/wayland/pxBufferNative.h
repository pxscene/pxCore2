// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxBufferNative.h

#ifndef PX_BUFFER_NATIVE_H
#define PX_BUFFER_NATIVE_H

#include <wayland-client.h>

#include "../pxCore.h"

// Structure used to describe a window surface under X11
typedef struct
{
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    struct wl_shell *shell;
    struct wl_seat *seat;
    struct wl_pointer *pointer;
    struct wl_surface *surface;
    struct wl_buffer* buffer;
    uint32_t* pixelData;
    int windowWidth;
    int windowHeight;
} pxSurfaceNativeDesc;

typedef pxSurfaceNativeDesc* pxSurfaceNative;

#endif
