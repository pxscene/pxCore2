#ifndef _WESTEROS_COMPOSITOR_H
#define _WESTEROS_COMPOSITOR_H

//#include "westeros-render.h"

typedef unsigned int uint32_t;
typedef int wl_fixed_t;

typedef struct _WstRect
{
   int x;
   int y;
   int width;
   int height;
} WstRect;

typedef struct _WstCompositor WstCompositor;

typedef enum _WstKeyboard_keyState
{
   WstKeyboard_keyState_released,
   WstKeyboard_keyState_depressed,
   WstKeyboard_keyState_none
} WstKeyboard_keyState;

typedef enum _WstKeyboad_modifiers
{
   WstKeyboard_shift= (1<<0),
   WstKeyboard_alt=   (1<<1),
   WstKeyboard_ctrl=  (1<<2),
   WstKeyboard_caps=  (1<<3)
} WstKeyboard_modifiers;

typedef enum _WstPointer_buttonState
{
   WstPointer_buttonState_released,
   WstPointer_buttonState_depressed
} WstPointer_buttonState;

typedef enum _WstClient_status
{
   WstClient_started,
   WstClient_stoppedNormal,
   WstClient_stoppedAbnormal,
   WstClient_connected,
   WstClient_disconnected
} WstClient_status;

typedef enum _WstHints
{
   WstHints_none= 0,
   WstHints_noRotation= (1<<0)
} WstHints;

typedef void (*WstTerminatedCallback)( WstCompositor *ctx, void *userData );
typedef void (*WstDispatchCallback)( WstCompositor *ctx, void *userData );
typedef void (*WstInvalidateSceneCallback)( WstCompositor *ctx, void *userData );
typedef void (*WstHidePointerCallback)( WstCompositor *ctx, bool hidePointer, void *userData );
typedef void (*WstClientStatus)( WstCompositor *ctx, int status, int clientPID, int detail, void *userData );

typedef void (*WstOutputHandleGeometryCallback)( void *userData, int32_t x, int32_t y, int32_t mmWidth, int32_t mmHeight,
                                                 int32_t subPixel, const char *make, const char *model, int32_t transform );
typedef void (*WstOutputHandleModeCallback)( void *userData, uint32_t flags, int32_t width, int32_t height, int32_t refreshRate );
typedef void (*WstOutputHandleDoneCallback)( void *UserData );
typedef void (*WstOutputHandleScaleCallback)( void *UserData, int32_t scale );

typedef void (*WstKeyboardHandleKeyMapCallback)( void *userData, uint32_t format, int fd, uint32_t size );
typedef void (*WstKeyboardHandleEnterCallback)( void *userData, struct wl_array *keys );
typedef void (*WstKeyboardHandleLeaveCallback)( void *userData );
typedef void (*WstKeyboardHandleKeyCallback)( void *userData, uint32_t time, uint32_t key, uint32_t state );
typedef void (*WstKeyboardHandleModifiersCallback)( void *userData, uint32_t mods_depressed, uint32_t mods_latched,
                                                    uint32_t mods_locked, uint32_t group );
typedef void (*WstKeyboardHandleRepeatInfoCallback)( void *userData, int32_t rate, int32_t delay );

typedef void (*WstPointerHandleEnterCallback)( void *userData, wl_fixed_t sx, wl_fixed_t sy );
typedef void (*WstPointerHandleLeaveCallback)( void *userData );
typedef void (*WstPointerHandleMotionCallback)( void *userData, uint32_t time, wl_fixed_t sx, wl_fixed_t sy );
typedef void (*WstPointerHandleButtonCallback)( void *userData, uint32_t time, uint32_t button, uint32_t state );
typedef void (*WstPointerHandleAxisCallback)( void *userData, uint32_t time, uint32_t axis, wl_fixed_t value );

typedef struct _WstOutputNestedListener
{
   WstOutputHandleGeometryCallback outputHandleGeometry;
   WstOutputHandleModeCallback outputHandleMode;
   WstOutputHandleDoneCallback outputHandleDone;
   WstOutputHandleScaleCallback outputHandleScale;
   
} WstOutputNestedListener;

typedef struct _WstKeyboardNestedListener
{
   WstKeyboardHandleKeyMapCallback keyboardHandleKeyMap;
   WstKeyboardHandleEnterCallback keyboardHandleEnter;
   WstKeyboardHandleLeaveCallback keyboardHandleLeave;
   WstKeyboardHandleKeyCallback keyboardHandleKey;
   WstKeyboardHandleModifiersCallback keyboardHandleModifiers;
   WstKeyboardHandleRepeatInfoCallback keyboardHandleRepeatInfo;
} WstKeyboardNestedListener;

typedef struct _WstPointerNestedListener
{
   WstPointerHandleEnterCallback pointerHandleEnter;
   WstPointerHandleLeaveCallback pointerHandleLeave;
   WstPointerHandleMotionCallback pointerHandleMotion;
   WstPointerHandleButtonCallback pointerHandleButton;
   WstPointerHandleAxisCallback pointerHandleAxis;
} WstPointerNestedListener;

/**
 * WestCompositorCreate
 *
 * Create a new compositor instance.  The caller should configure
 * the instance with WstCompositorSet* calls and then start the
 * compositor operation by calling WstCompositorStart.
 */
WstCompositor* WstCompositorCreate();

/** 
 * WstCompositorDestroy
 *
 * Destroy a compositor instance.  If the compositor is running
 * it will be stopped, and then all resources will be freed.
 */
void WstCompositorDestroy( WstCompositor *ctx );

/**
 * WstCompositorGetLastErrorDetail
 *
 * Returns a null terminated string giving information about the
 * last error that has occurred.
 */
const char *WstCompositorGetLastErrorDetail( WstCompositor *ctx );

/**
 * WstCompositorSetDisplayName
 *
 * Specify the name of the wayland display that this instance will
 * create.  This must be called prior to WstCompositorStart.  If not
 * called, the behaviour is as follows: for a nested compositor a
 * display name will be generated, for a non-nested compositor the
 * default display name of 'wayland-0' will be used.  The display
 * name of a compositor can be obtained using WstCompositorGetDisplayName.
 */
bool WstCompositorSetDisplayName( WstCompositor *ctx, const char *displayName );

/**
 * WstCompositorSetFrameRate
 *
 * Specity the rate in frames per second (fps) that the compositor should
 * generate each new composited output frame.  This can be called at any time.
 */
bool WstCompositorSetFrameRate( WstCompositor *ctx, unsigned int frameRate );

/**
 * WstCompositorSetNativeWindow
 *
 * Specify the native window to be used by the compositor render module
 * in creating its rendering context.
 */
bool WstCompositorSetNativeWindow( WstCompositor *ctx, void *nativeWindow );

/**
 * WstCompositorSetRendererModule
 *
 * Specify the name of the module the compositor will use for rendering.  This
 * will be a shared library file name without path.  An example module 
 * name might be libwesteros_render_gl.so.0.  This must be called prior
 * to WstCompositorStart.
 */
bool WstCompositorSetRendererModule( WstCompositor *ctx, const char *rendererModule );

/**
 * WstCompositorSetIsNested
 *
 * Specify if the compositor is to act as a nested compositor.  When acting
 * as a nested compositor, the compositor will create a wayland display that
 * clients can connect and render to, but the compositor will act as a client
 * to another compositor and its output frames will be drawn to a surface
 * of the second compositor.
 */
bool WstCompositorSetIsNested( WstCompositor *ctx, bool isNested );

/**
 * WstCompositorSetIsRepeater
 *
 * Specify if the compositor is to act as a repeating nested compositor.  A 
 * normal nested compositor will compose client surfaces to produce an output
 * surface which is then sent to a second compositor for display.  A repeating 
 * nested compositor will not perform any composition rendering but instead 
 * will forward surface buffers from its clients to the wayland display to 
 * which it is connected.  Enabling repeating will also enable nested 
 * composition.
 */
bool WstCompositorSetIsRepeater( WstCompositor *ctx, bool isRepeater );

/**
 * WstCompositorSetIsEmbedded
 *
 * Specify if the compositor is to act as an embedded compositor.  When acting
 * as an embedded compositor, the compositor will create a wayland display that
 * clients can connect and render to, but the compositor will only compose
 * its scene when WstCompositorComposeEmbedded is called.  An embedded
 * compositor should use libwesteros_render_embedded.so.0 as its
 * renderer module (or some other module that supports embedded composition).
 */
bool WstCompositorSetIsEmbedded( WstCompositor *ctx, bool isEmbedded );

/**
 * WstCompositorSetOutputSize
 *
 * Specify the size of the output surface for the compositor.  This
 * must be called prior to WstCompositorStart
 */
bool WstCompositorSetOutputSize( WstCompositor *ctx, int width, int height );

/**
 * WstCompositorSetNestedDisplayName
 *
 * Specify the wayland display name that this compositor instance should connect
 * and render to as a nested compositor.  This must be called prior to 
 * WstCompositorStart.
 */
bool WstCompositorSetNestedDisplayName( WstCompositor *ctx, const char *nestedDisplayName );

/**
 * WstCompositorSetNestedSize
 *
 * Specify the size of the surface which should be created on the display
 * specified with WstCompositorSetNestedDisplayName in which to display the
 * composited output.  This must be called prior to WstCompositorStart.
 */
bool WstCompositorSetNestedSize( WstCompositor *ctx, unsigned int width, unsigned int height );

/**
 * WstCompositorSetAllowCursorModification
 *
 * Specify whether compositor clients are permitted to modify the pointer cursor
 * image.  This must be called prior to WstCompositorStart.
 */
bool WstCompositorSetAllowCursorModification( WstCompositor *ctx, bool allow );
 
/**
 * WstCompositorGetDisplayName
 *
 * Obtain the display name used by this compositor instance.  This will 
 * be the name set prior to start via WstCompositorSetDisplayName or, for
 * a nested compositor for which no name was specified, the display name
 * that was automatically generated.  This can be called at any time.
 */
const char *WstCompositorGetDisplayName( WstCompositor *ctx );

/**
 * WstCompositorGetFrameRate
 *
 * Obtain the current output frame rate being used by the 
 * compositor instance.  The returned value will be in 
 * frames per second (fps).  This can be called at any time.
 */
unsigned int WstCompositorGetFrameRate( WstCompositor *ctx );

/**
 * WstCompositorGetRendererModule
 *
 * Obtain the name of the renderer module being used by
 * this compositor instance.  This can be called at any time.
 */
const char *WstCompositorGetRenderModule( WstCompositor *ctx );

/**
 * WstCompositorGetIsNested
 *
 * Determine if this compsitor instance is acting as a nested
 * compositor or not.  This may be called at any time.
 */
bool WstCompositorGetIsNested( WstCompositor *ctx );

/**
 * WstCompositorGetIsRepeater
 *
 * Determine if this compsitor instance is acting as a repeating 
 * nested compositor or not.  This may be called at any time.
 */
bool WstCompositorGetIsRepeater( WstCompositor *ctx );

/**
 * WstCompositorGetIsEmbedded
 *
 * Determine if this compsitor instance is acting as an embedded
 * compositor or not.  This may be called at any time.
 */
bool WstCompositorGetIsEmbedded( WstCompositor *ctx );

/**
 * WstCompositorGetOutputSize
 *
 * Obtain the width and height of the compositor output.
 */
void WstCompositorGetOutputSize( WstCompositor *ctx, unsigned int *width, unsigned int *height );

/**
 * WstCompositorGetNestedDisplayName
 *
 * Obtain the name of the wayland display that this compositor
 * instance will be, or is using to connect to as a nested
 * compositor.  This can be called at any time.
 */
const char *WstCompositorGetNestedDisplayName( WstCompositor *ctx );

/**
 * WstCompositorGetNestedSize
 *
 * Obtain the size of surface this compositor instance will create
 * or has created on another wayland display as a nested compositor.
 * This can be called at any time.
 */
void WstCompositorGetNestedSize( WstCompositor *ctx, unsigned int *width, unsigned int *height );

/**
 * WstCompositorGetAllowCursorModification
 *
 * Determine if this compsitor instance is configured to allow
 * compositor clients to modify the pointer cursor image.  
 * This may be called at any time.
 */
bool WstCompositorGetAllowCursorModification( WstCompositor *ctx );

/**
 * WstCompositorSetTerminatedCallback
 *
 * Specifies a callback for an embedded compositor to invoke to signal that it
 * has terminated.
 */
bool WstCompositorSetTerminatedCallback( WstCompositor *ctx, WstTerminatedCallback cb, void *userData );

/**
 * WstCompositorSetDispatchCallback
 *
 * Specifies a callback for a compositor to periodically invoke to give an opportunity for any required
 * implementatipn specific event dispatching or other 'main loop' type processing.
 */
bool WstCompositorSetDispatchCallback( WstCompositor *ctx, WstDispatchCallback cb, void *userData );

/**
 * WstCompositorSetInvalidateCallback
 *
 * Specifies a callback for an embedded compositor to invoke to signal that its
 * scene has become invalid and that WstCompositorComposeEmbedded should be called.
 */
bool WstCompositorSetInvalidateCallback( WstCompositor *ctx, WstInvalidateSceneCallback cb, void *userData );

/**
 * WstCompositorSetHidePointerCallback
 *
 * Specifies a callback for an embedded compositor to invoke to signal that any
 * pointer image being displayed by the process embedding this compositor should be
 * hidden or shown.  The embedded compositor will request the host cursor be hidden
 * when a client requests a different pointer be used.
 */
bool WstCompositorSetHidePointerCallback( WstCompositor *ctx, WstHidePointerCallback cb, void *userData );

/**
 * WstCompositorSetClientStatusCallback
 *
 * Specifies a callback for an embedded compositor to invoke to signal the status of a
 * client process.  The callback will supply a status value from the WstClient_status
 * enum and the client pid.  If the status is WstClient_status_stoppedAbnormal the detail
 * value will be the signal that caused the client to terminate.
 */
bool WstCompositorSetClientStatusCallback( WstCompositor *ctx, WstClientStatus cb, void *userData );

/**
 * WstCompositorSetOutputNestedListener
 *
 * Specifies a set of callbacks to be invoked by a nested compositor for output events.  By default
 * the nested compositor will forward output events to a connected client.  When a listener is set
 * using WstCompositorSetOutputNestedListener the events will instead be passed to the caller
 * through the specified callback functions.  This allows the caller to handle the events outside
 * of Wayland.  This must be called prior to WstCompositorStart.
 */
bool WstCompositorSetOutputNestedListener( WstCompositor *ctx, WstOutputNestedListener *listener, void *userData );

/**
 * WstCompositorSetKeyboardNestedListener
 *
 * Specifies a set of callbacks to be invoked by a nested compositor for keyboard input.  By default
 * the nested compositor will forward keyboard events to a connected client.  When a listener is set
 * using WstCompositorSetKeyboardNestedListener the events will instead be passed to the caller
 * through the specified callback functions.  This allows the caller to route keyboard input outside
 * of Wayland.  This must be called prior to WstCompositorStart.
 */
bool WstCompositorSetKeyboardNestedListener( WstCompositor *ctx, WstKeyboardNestedListener *listener, void *userData );

/**
 * WstCompositorSetPointerNestedListener
 *
 * Specifies a set of callbacks to be invoked by a nested compositor for pointer input.  By default
 * the nested compositor will forward pointer events to a connected client.  When a listener is set
 * using WstCompositorSetPointerNestedListener the events will instead be passed to the caller
 * through the specified callback functions.  This allows the caller to route pointer input outside
 * of Wayland.  This must be called prior to WstCompositorStart.
 */
bool WstCompositorSetPointerNestedListener( WstCompositor *ctx, WstPointerNestedListener *listener, void *userData );

/**
 * WstCompositorComposeEmbedded
 *
 * Requests that the current scene be composed as part of the configured embedded environment.  This
 * should be called with the environment setup for offscreen rendering.  For example, if OpenGL is
 * being used for rendering, WstCompositorComposeEmbedded should be called with an FBO set as the
 * current render target.
 *
 * The x, y, width and height give the desired composition rectangle.  The matrix and alpha
 * values are what the caller intends to apply when the composited scene is subsequently rendered
 * from the offscreen target to the callers scene.  Based on the hinting provided, the compositor will
 * either render to the offscreen target or use an available fast path to render to a separate 
 * plane.  If no fast path is available it will render to the offscreen target without the transform
 * and alpha.  If it renders to a separate plane it will apply the provide transformation matrix and alpha
 * and will set needHolePunch to true.  Upon return, if needHolePunch is true, the caller should 
 * render a hole punch for each rectangle returned in rects, otherwise it should render the offscreen 
 * target to its scene while applying the transformation matrix and alpha.
 *
 * This should only be called while the compositor is running.
 */
bool WstCompositorComposeEmbedded( WstCompositor *ctx, 
                                   int x, int y, int width, int height,
                                   float *matrix, float alpha, 
                                   unsigned int hints, 
                                   bool *needHolePunch, std::vector<WstRect> &rects );

/**
 * WstCompositorStart
 *
 * Start the compositor operating.  This will cause the compositor to create its
 * wayland display, connect to its target wayland display if acting as a nested 
 * compositor, and start processing events.  The function is not blocking and will
 * return as soon as the compositor is operating.
 */
bool WstCompositorStart( WstCompositor *ctx );

/**
 * WstCompositorStop
 *
 * Stops the operation of a compositor.  The compositor will halt all operation
 * and release all resources.
 */
void WstCompositorStop( WstCompositor *ctx );

/**
 * WstCompositorKeyEvent
 *
 * Pass a key event to the compositor.  The compositor will route the event
 * to an appropriate compositor client.
 */
void WstCompositorKeyEvent( WstCompositor *ctx, int keyCode, unsigned int keyState, unsigned int modifiers );

/**
 * WstCompositorPointerEnter
 *
 * Notifiy compositor that the pointer has entered its bounds.
 */
void WstCompositorPointerEnter( WstCompositor *ctx );

/**
 * WstCompositorPointerLeave
 *
 * Notifiy compositor that the pointer has exited its bounds.
 */
void WstCompositorPointerLeave( WstCompositor *ctx );

/**
 * WstCompositorPointerMoveEvent
 *
 * Pass a pointer move event to the compositor.  Th compositor will route the event
 * to an appropriate compositor client.
 */
void WstCompositorPointerMoveEvent( WstCompositor *ctx, int x, int y );

/**
 * WstCompositorPointerButtonEvent
 *
 * Pass a pointer button event to the compositor.  The compositor will route the event
 * to an appropriate compositor client.
 */
void WstCompositorPointerButtonEvent( WstCompositor *ctx, unsigned int button, unsigned int buttonState );


/**
 * WstCompositorLaunchClient
 *
 * Launch a named process intended to connect to the compositor as a client.  This should only be called
 * while the compositor is running.  The function is blocking and will not return until the client
 * process terminates or fails to launch.
 */
bool WstCompositorLaunchClient( WstCompositor *ctx, const char *cmd );

#endif

