// pxCore CopyRight 2005-2007 John Robinson
// Portable Framebuffer and Windowing Library
// pwWindowNative.cpp

#include <iostream>

#include "pxWindow.h"
#include "pxKeycodes.h"
#include "pxWindowNative.h"
#include "../pxWindowUtil.h"

#import <Cocoa/Cocoa.h>

//
// NOTE:   'PX_OPENGL' *and* 'GLGL' are needed for *pxScene2d* builds for OpenGL
//

//
// NOTE:   'PX_OPENGL' *and* 'GLGL' are NOT needed for other example builds.
//
#define PX_OPENGL
#define GLGL

#ifdef GLGL

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#endif

#include "CoreFoundation/CoreFoundation.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<pxWindowNative*> pxWindowNative::sWindowVector;
void pxWindowNative::registerWindow(pxWindowNative* win)
{
  pxWindowNative::sWindowVector.push_back(win);
}

void pxWindowNative::unregisterWindow(pxWindowNative* win)
{
  std::vector<pxWindowNative*>::iterator i = std::find(pxWindowNative::sWindowVector.begin(), pxWindowNative::sWindowVector.end(), win);
  if (i != pxWindowNative::sWindowVector.end())
    pxWindowNative::sWindowVector.erase(i);
}

void pxWindowNative::closeAllWindows()
{
  for (int window=0; window<pxWindowNative::sWindowVector.size(); window++)
  {
    pxWindowNative::_helper_onCloseRequest(pxWindowNative::sWindowVector[window]);
  }
  pxWindowNative::sWindowVector.clear();
}

pxWindowNative::~pxWindowNative()
{
  unregisterWindow(this);
}

@interface WinDelegate : NSObject <NSWindowDelegate>
@end

@implementation WinDelegate
{
  pxWindowNative* mWindow;
}

-(id)initWithPXWindow:(pxWindowNative*)window
{
  if(self = [super init])
  {
    mWindow = window;

    // --------------------------------------------------------------------------------------------------------------------
    // This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the App Bundle

    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef  resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    
    char resourcesBundlePath[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)resourcesBundlePath, PATH_MAX))
    {
        // error!
        NSLog(@"ERROR: CFURLGetFileSystemRepresentation() - failed !");
    }
    CFRelease(resourcesURL);

    //
    // Xcode DEBUG builds package the App Bundle a little differently.
    //
    NSString *init_js = [NSString stringWithFormat:@"%s/init.js", resourcesBundlePath];
    
    BOOL isXCodeBuild = [ [NSFileManager defaultManager] fileExistsAtPath: init_js isDirectory: nil];
    
    if(isXCodeBuild)
    {
      chdir(resourcesBundlePath); 

      char *value = resourcesBundlePath;

      char *key = (char *) "NODE_PATH";
      char *val = (char *) getenv(key); // existing

      NSLog(@"NODE_PATH:  [ %s ]", val);
    
      // Set NODE_PATH env
      int overwrite = 1;
      setenv(key, value, overwrite);
      
      NSLog(@"NODE_PATH:  [ %s ]", val);
    }
  
    // --------------------------------------------------------------------------------------------------------------------
    
    return self;
  }
  
  return nil;
}

- (void)windowDidEnterFullScreen:(NSNotification *)notification
{
  NSSize s = [[[notification object] contentView] frame].size;
  pxWindowNative::_helper_onSize(mWindow, s.width, s.height);  
}

- (void)windowDidExitFullScreen:(NSNotification *)notification
{
  NSSize s = [[[notification object] contentView] frame].size;
  pxWindowNative::_helper_onSize(mWindow, s.width, s.height);
}

- (void)windowDidResize: (NSNotification*)notification
{
  NSSize s = [[[notification object] contentView] frame].size;
  pxWindowNative::_helper_onSize(mWindow, s.width, s.height);
}

- (BOOL)windowShouldClose:(id)sender
{
  if (mWindow)
    pxWindowNative::_helper_onCloseRequest(mWindow);
  return FALSE;
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1

@interface MyView : NSView <NSDraggingDestination, NSPasteboardItemDataProvider>

- (IBAction)cut:   sender;
- (IBAction)copy:  sender;
- (IBAction)paste: sender;

@end

#ifdef GLGL
NSOpenGLContext *openGLContext;
#endif //GLGL

@interface MyView()
{
#ifdef GLGL
  NSOpenGLPixelFormat *pixelFormat;
  
  GLint virtualScreen;
  BOOL enableMultisample;
  //CVDisplayLinkRef displayLink;
#endif
  pxWindow* mWindow;
  NSTrackingArea* trackingArea;
}

@end

@implementation MyView

#if 0
- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
{
  // There is no autorelease pool when this method is called
  // because it will be called from a background thread
  // It's important to create one or you will leak objects
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  
  [self drawView];
  
  [pool release];
  return kCVReturnSuccess;
}
#endif


#ifdef GLGL
- (NSOpenGLContext*)openGLContext
{
  return openGLContext;
}

- (NSOpenGLPixelFormat*)pixelFormat
{
  return pixelFormat;
}

- (void)lockFocus
{
  [super lockFocus];
  
  if ([[self openGLContext] view] != self)
  {
    // Unlike NSOpenGLView, NSView does not export a -prepareOpenGL method to override.
    // We need to call it explicitly.
    [self prepareOpenGL];
    
    [[self openGLContext] setView:self];
  }
}
#endif

#ifdef GLGL
- (id)initWithFrame:(NSRect)frame
{
  // Any Macintosh system configuration that includes more GPUs than displays will have both online and offline GPUs.
  // Online GPUs are those that are connected to a display while offline GPUs are those that have no such output
  // hardware attached. In these configurations you may wish to take advantage of the offline hardware, or to be able
  // to start rendering on this hardware should a display be connected at a future date without having to reconfigure
  // and reupload all of your OpenGL content.
  //
  // To enable the usage of offline renderers, add NSOpenGLPFAAllowOfflineRenderers when using NSOpenGL or
  // kCGLPFAAllowOfflineRenderers when using CGL to the attribute list that you use to create your pixel format.
  NSOpenGLPixelFormatAttribute attribs[] =
  {
    /*NSOpenGLPFADoubleBuffer,*/
    NSOpenGLPFAAllowOfflineRenderers, // lets OpenGL know this context is offline renderer aware
    NSOpenGLPFAMultisample, 1,
    NSOpenGLPFASampleBuffers, 1,
    NSOpenGLPFASamples, 4,
    NSOpenGLPFAColorSize, 32,
    NSOpenGLPFAOpenGLProfile,NSOpenGLProfileVersionLegacy/*, NSOpenGLProfileVersion3_2Core*/, // Core Profile is the future
    0
  };
  
  NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
  if(!pf)
  {
    NSLog(@"Failed to create pixel format.");
    [self release];
    return nil;
  }
  
  self = [super initWithFrame:frame];
  if (self)
  {
    pixelFormat   = [pf retain];
    openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    
    enableMultisample = YES;
      
    // Register for Drag'n'Drop events
    [self registerForDraggedTypes:@[NSFilenamesPboardType,
                                    NSPasteboardTypeString,
                                    NSURLPboardType
                                    ]];
  }
  
  [pf release];
  
  [self initGL];
  
  return self;
}

- (void)initGL
{
  [[self openGLContext] makeCurrentContext];
  
  // Synchronize buffer swaps with vertical refresh rate
  GLint one = 1;
  [[self openGLContext] setValues:&one forParameter:NSOpenGLCPSwapInterval];
  
  if(enableMultisample)
    glEnable(GL_MULTISAMPLE);
}
#endif


#if 0
// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    CVReturn result = [(MyView*)displayLinkContext getFrameForTime:outputTime];
    return result;
}

-(void)setupDisplayLink
{
  // Create a display link capable of being used with all active displays
  CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
  
  // Set the renderer output callback function
  CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, self);
  
  // Set the display link for the current renderer
  CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
  CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
  CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
  
  // Activate the display link
  CVDisplayLinkStart(displayLink);
}
#endif

#ifdef GLGL
- (void)prepareOpenGL
{
  // Make the OpenGL context current and do some one-time initialization.
  //  [self initGL];
  
#if 0
  // Create the CVDisplayLink for driving the rendering loop
  [self setupDisplayLink];
#endif
  
  // This is an NSView subclass, not an NSOpenGLView.
  // We need to register the following notifications to be able to detect renderer changes.
  
  // Add an observer to NSViewGlobalFrameDidChangeNotification, which is posted
  // whenever an NSView that has an attached NSSurface changes size or changes screens
  // (thus potentially changing graphics hardware drivers).
  [[NSNotificationCenter defaultCenter] addObserver: self
                                           selector: @selector(surfaceNeedsUpdate:)
                                               name: NSViewGlobalFrameDidChangeNotification
                                             object: self];
  
  // Also register for changes to the display configuation using Quartz Display Services
  CGDisplayRegisterReconfigurationCallback(MyDisplayReconfigurationCallBack, self);
}

- (void)update
{
  // Call -[NSOpenGLContext update] to let it handle screen selection after resize/move.
  [[self openGLContext] update];
  
  // A virtual screen change is detected
  if(virtualScreen != [[self openGLContext] currentVirtualScreen])
  {
    // Find the current renderer and update the UI.
    [self gpuChanged];
    
    // Add your additional handling here
    // Adapt to any changes in capabilities
    // (such as max texture size, extensions and hardware capabilities such as the amount of VRAM).
  }
}

- (void)surfaceNeedsUpdate:(NSNotification *)notification
{
  [self update];
}

// When displays are reconfigured this callback will be called. You can take this opportunity to do further
// processing or pass the notification on to an object for further handling.
void MyDisplayReconfigurationCallBack(CGDirectDisplayID display,
                                      CGDisplayChangeSummaryFlags flags,
                                      void *userInfo)
{
  if (flags & kCGDisplaySetModeFlag)
  {
    // In this example we've passed 'self' for the userInfo pointer,
    // so we can cast it to an appropriate object type and forward the message onwards.
    [((MyView *)userInfo) update];
    
    // Display has been reconfigured.
    // Adapt to any changes in capabilities
    // (such as max texture size, extensions and hardware capabilities such as the amount of VRAM).
  }
}

- (void)gpuChanged
{
  virtualScreen = [[self openGLContext] currentVirtualScreen];
  
  // Since this may be called from outside the display loop, make sure
  // our context is current so the GL calls all work properly.
  [[self openGLContext] makeCurrentContext];
  
#if 0
  // Update UI elements with current renderer name, etc.
  [rendererNameField setStringValue:[NSString stringWithCString:(const char *)
                                     glGetString(GL_RENDERER) encoding:NSASCIIStringEncoding]];
#endif
  
  // Use the current virtual screen index to interrogate the pixel format
  // for its display mask and renderer id.
  GLint displayMask;
  GLint rendererID;
  [pixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
  [pixelFormat getValues:&rendererID  forAttribute:NSOpenGLPFARendererID forVirtualScreen:virtualScreen];
  
  // The software renderer (for whatever reason) returns a display mask of all 0's rather than
  // all 1's, so we swap that out here so that CGLQueryRendererInfo will at least find the software
  // renderer.
  if(displayMask == 0)
    displayMask = 0xffffffff;
  
  // Get renderer info for all renderers that match the display mask.
  GLint i, nrend = 0;
  CGLRendererInfoObj rend;
  
  CGLQueryRendererInfo((GLuint)displayMask, &rend, &nrend);
  
  GLint videoMemory = 0;
  for(i = 0; i < nrend; i++)
  {
    GLint thisRenderer;
    
    CGLDescribeRenderer(rend, i, kCGLRPRendererID, &thisRenderer);
    
    // See if this is the one we want
    if(thisRenderer == rendererID)
      CGLDescribeRenderer(rend, i, kCGLRPVideoMemoryMegabytes, &videoMemory);
  }
  
  CGLDestroyRendererInfo(rend);
  
#if 0
  // Update UI
  [videoRAMField setStringValue:[NSString stringWithFormat:@"%d MB", videoMemory]];
#endif
}

#endif

- (void)drawView
{
  //GLGL
#ifdef GLGL
  [[self openGLContext] makeCurrentContext];
  
  // We draw on a secondary thread through the display link
  // Add a mutex around to avoid the threads from accessing the context simultaneously
  CGLLockContext([[self openGLContext] CGLContextObj]);
  
  glViewport(0,0,[self bounds].size.width,[self bounds].size.height);
  
  glClearColor(0.675f,0.675f,0.675f,1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
#endif
  
  
#if 0
  if (!renderer) //first time drawing
  {
    // Create a BoingRenderer object which handles the rendering of a Boing ball.
    renderer = [[BoingRenderer alloc] init];
    
    // Delegate to the BoingRenderer object to create an Orthographic projection camera
    [renderer makeOrthographicForWidth:self.bounds.size.width height:self.bounds.size.height];
    
    // Update the text fields with the initial renderer info
    [self gpuChanged];
  }
  
  // Let the BoingRenderer object update the physics stuff
  [renderer update];
  
  // Delegate to the BoingRenderer object for drawing the boling ball
  [renderer render];
#endif
  
  //GLGL
#ifdef GLGL
  [[self openGLContext] flushBuffer];
  
  CGLUnlockContext([[self openGLContext] CGLContextObj]);
#endif
}

#ifdef GLGL
- (void)dealloc
{
#if 0
  // Stop the display link BEFORE releasing anything in the view
  // otherwise the display link thread may call into the view and crash
  // when it encounters something that has been released
  CVDisplayLinkStop(displayLink);
  
  CVDisplayLinkRelease(displayLink);
#endif
  
  // Destroy the GL context AFTER display link has been released
  //[renderer release];
  [pixelFormat release];
  [openGLContext release];
  
  // Remove the registrations
  [[NSNotificationCenter defaultCenter] removeObserver:self
                                                  name:NSViewGlobalFrameDidChangeNotification
                                                object:self];
  
  CGDisplayRemoveReconfigurationCallback(MyDisplayReconfigurationCallBack, self);
  
  [super dealloc];
}
#endif

- (BOOL)isOpaque
{
  return YES;
}


-(id)initWithPXWindow:(pxWindow*)window
{
  if(self = [super init])
  {
    mWindow = window;
    //[self updateTrackingAreas];
    return self;
  }
  return nil;
}

-(BOOL)isFlipped
{
  return YES;
}

#if 0
 #ifdef PX_OPENGL
 -(void)prepareOpenGL
 {
   pxWindowNative::_helper_onCreate(mWindow);
 }
 #endif
#endif


#pragma mark - Cut and Paste Frist Responders


- (IBAction)newWindow:sender
{
  NSString     *path = [[NSBundle mainBundle] bundlePath];
  NSWorkspace*   ws  = [NSWorkspace sharedWorkspace];
  NSURL*         url = [NSURL fileURLWithPath:path isDirectory:NO];

  [ws launchApplicationAtURL: url
                     options: NSWorkspaceLaunchNewInstance
               configuration: @{}
                       error: nil];
}

- (IBAction)toggleAddressBar:sender
{
  pxWindowNative::_helper_onKeyDown(mWindow, PX_KEY_F, PX_MOD_CONTROL | PX_MOD_ALT);  // Fake a CTRL-ALT-F
}

- (IBAction)cut:sender
{
  pxWindowNative::_helper_onKeyDown(mWindow, PX_KEY_X, PX_MOD_CONTROL);  // Fake a CTRL-X
}

- (IBAction)copy:sender
{
    pxWindowNative::_helper_onKeyDown(mWindow, PX_KEY_C, PX_MOD_CONTROL);  // Fake a CTRL-C
}

- (IBAction)paste:sender
{
    pxWindowNative::_helper_onKeyDown(mWindow, PX_KEY_V, PX_MOD_CONTROL);  // Fake a CTRL-V
}

#pragma mark - Mouse Operations

-(void)mouseDown:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"mouseDown: %f, %f", p.x, p.y);

  uint32_t flags = PX_LEFTBUTTON;
  
  if (event.modifierFlags & NSShiftKeyMask)     flags |= PX_MOD_SHIFT;
  if (event.modifierFlags & NSControlKeyMask)   flags |= PX_MOD_CONTROL;
  if (event.modifierFlags & NSAlternateKeyMask) flags |= PX_MOD_ALT;
  if (event.modifierFlags & NSCommandKeyMask)   flags |= PX_MOD_COMMAND;
  
  pxWindowNative::_helper_onMouseDown(mWindow, p.x, p.y, flags);
}

-(void)mouseUp:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"mouseUp: %f, %f", p.x, p.y);

  uint32_t flags = PX_LEFTBUTTON;
  
  if (event.modifierFlags & NSShiftKeyMask)     flags |= PX_MOD_SHIFT;
  if (event.modifierFlags & NSControlKeyMask)   flags |= PX_MOD_CONTROL;
  if (event.modifierFlags & NSAlternateKeyMask) flags |= PX_MOD_ALT;
  if (event.modifierFlags & NSCommandKeyMask)   flags |= PX_MOD_COMMAND;
  
  pxWindowNative::_helper_onMouseUp(mWindow, p.x, p.y, flags);
}

-(void)rightMouseDown:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"rightMouseDown: %f, %f", p.x, p.y);

  uint32_t flags = PX_RIGHTBUTTON;
  
  if (event.modifierFlags & NSShiftKeyMask)     flags |= PX_MOD_SHIFT;
  if (event.modifierFlags & NSControlKeyMask)   flags |= PX_MOD_CONTROL;
  if (event.modifierFlags & NSAlternateKeyMask) flags |= PX_MOD_ALT;
  if (event.modifierFlags & NSCommandKeyMask)   flags |= PX_MOD_COMMAND;

  pxWindowNative::_helper_onMouseDown(mWindow, p.x, p.y, flags);
}

-(void)rightMouseUp:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"rightMouseUp: %f, %f", p.x, p.y);

  uint32_t flags = PX_RIGHTBUTTON;
  
  if (event.modifierFlags & NSShiftKeyMask)     flags |= PX_MOD_SHIFT;
  if (event.modifierFlags & NSControlKeyMask)   flags |= PX_MOD_CONTROL;
  if (event.modifierFlags & NSAlternateKeyMask) flags |= PX_MOD_ALT;
  if (event.modifierFlags & NSCommandKeyMask)   flags |= PX_MOD_COMMAND;

  pxWindowNative::_helper_onMouseUp(mWindow, p.x, p.y, flags);
}

-(void)mouseMoved:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  //NSLog(@"mouseMoved: %f, %f", p.x, p.y);
  pxWindowNative::_helper_onMouseMove(mWindow, p.x, p.y);
}

- (void)scrollWheel:(NSEvent *)event
{
  pxWindowNative::_helper_onScrollWheel(mWindow, event.deltaX, event.deltaY);
}

-(void)keyDown:(NSEvent*)event
{
  uint32_t flags = 0;

  //NSLog(@"keyDown, repeat:%s", event.ARepeat?"YES":"NO");
  //if (!event.ARepeat) 
  {
    // send px key down
    if (event.modifierFlags & NSShiftKeyMask)     flags |= PX_MOD_SHIFT;
    if (event.modifierFlags & NSControlKeyMask)   flags |= PX_MOD_CONTROL;
    if (event.modifierFlags & NSAlternateKeyMask) flags |= PX_MOD_ALT;
    if (event.modifierFlags & NSCommandKeyMask)   flags |= PX_MOD_COMMAND;

    if (event.ARepeat)
      flags |= PX_KEYDOWN_REPEAT;

    pxWindowNative::_helper_onKeyDown(mWindow, keycodeFromNative(event.keyCode), flags);
  }
  
  NSString* s = event.characters;
  if (![s isEqualToString:@""])
  {
    uint32_t c = [s characterAtIndex:0];
    // filter out control characters
    // TODO overfiltering... look at IMEs and unicode key input
    // NOTE that iscntrl does not filter out non-printable values like up/down arrows
    if (!iscntrl(c) && c < 128 &&
        ( (flags & PX_MOD_COMMAND) != PX_MOD_COMMAND) )
      pxWindowNative::_helper_onChar(mWindow, c);

  }
}

-(void)keyUp:(NSEvent*)event
{
  uint32_t flags = 0;
  
  if (event.modifierFlags & NSShiftKeyMask)     flags |= PX_MOD_SHIFT;
  if (event.modifierFlags & NSControlKeyMask)   flags |= PX_MOD_CONTROL;
  if (event.modifierFlags & NSAlternateKeyMask) flags |= PX_MOD_ALT;
  if (event.modifierFlags & NSCommandKeyMask)   flags |= PX_MOD_COMMAND;
  
  pxWindow::_helper_onKeyUp(mWindow,keycodeFromNative(event.keyCode), flags);
}

-(void)mouseEntered:(NSEvent*)event
{
  NSLog(@"mouseEntered");
  pxWindowNative::_helper_onMouseEnter(mWindow);
}

-(void)mouseExited:(NSEvent*)event
{
  NSLog(@"mouseExited");
  pxWindowNative::_helper_onMouseLeave(mWindow);
}

-(void)mouseDragged:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"mouseDragged: %f, %f", p.x, p.y);
  // send as px mousemove
  pxWindowNative::_helper_onMouseMove(mWindow, p.x, p.y);
}

-(void)rightMouseDragged:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"rightMouseDragged: %f, %f", p.x, p.y);
  // send as px mousemove
  pxWindowNative::_helper_onMouseMove(mWindow, p.x, p.y);
}

-(void)fireAnimationTimer:(NSTimer*)timer
{
  pxWindowNative::_helper_onAnimationTimer(mWindow);
}

-(void)updateTrackingAreas
{
  if(trackingArea != nil)
  {
    [self removeTrackingArea:trackingArea];
    [trackingArea release];
  }
  
  int opts = (NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInActiveApp |NSTrackingEnabledDuringMouseDrag);
  trackingArea = [ [NSTrackingArea alloc] initWithRect:[self bounds]
                                               options:opts
                                                 owner:self
                                              userInfo:nil];
  [self addTrackingArea:trackingArea];
}


-(void)drawRect:(NSRect)dirtyRect
{
#ifdef GLGL
 #if 0
    [self drawView];
 #else
  [[self openGLContext] makeCurrentContext];
 #endif
#endif
  
  pxWindowNative::_helper_onDraw(mWindow, (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort]);
  
#ifdef GLGL
#ifdef PX_OPENGL
  glFlush();
#endif
  //[[self openGLContext] flushBuffer];
#endif

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma mark - Dragging Operations

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
    NSPasteboard           *pboard = [sender draggingPasteboard];
    
    /*------------------------------------------------------
     method called whenever a drag enters our drop zone
     --------------------------------------------------------*/
    
    if ( pboard )
    {
      if( [[pboard types] containsObject:NSPasteboardTypeString] )
      {
        NSLog(@" Got STRING ");
        
        if (sourceDragMask & NSDragOperationLink)
        {
            return NSDragOperationLink;
        }
        else if (sourceDragMask & NSDragOperationCopy)
        {
            return NSDragOperationCopy;
        }
      }
      else
      if( [[pboard types] containsObject:NSFilenamesPboardType] )
      {
          if (sourceDragMask & NSDragOperationLink)
          {
              return NSDragOperationLink;
          }
          else if (sourceDragMask & NSDragOperationCopy)
          {
              return NSDragOperationCopy;
          }
        }
    }
    
    return NSDragOperationNone;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (void)draggingExited:(id <NSDraggingInfo>)sender
{
    /*------------------------------------------------------
     method called whenever a drag exits our drop zone
     --------------------------------------------------------*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
    /*------------------------------------------------------
     method to determine if we can accept the drop
     --------------------------------------------------------*/
    
    return YES;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    /*------------------------------------------------------
     method that should handle the drop data
     --------------------------------------------------------*/
    if ( [sender draggingSource] != self )
    {
      pxClipboardNative *clip = pxClipboardNative::instance();
      
      if(clip == nil)
      {
        NSLog(@"ERROR: performDragOperation() - Failed to get Clipboard instance");
        return NO;
      }

      NSURL *fileURL = [NSURL URLFromPasteboard: [sender draggingPasteboard]];

      if(fileURL)
      {
        // Handle Drag'n'Dropped >> FILE
        //
        NSString  *filePath = [fileURL path];
        NSString  *dropURL  = [NSString stringWithFormat:@"file://%@", filePath ];

        if(dropURL)
        {
            clip->setString("TEXT", [dropURL UTF8String]);

            pxWindowNative::_helper_onKeyDown(mWindow, 65, 16);  // Fake a CTRL-A ... select ALL to replace current URL
        }
      }
      else
      {
        // Handle Drag'n'Dropped >> TEXT
        //
        NSString *text = [[sender draggingPasteboard] stringForType:NSPasteboardTypeString];

        if(text)
        {
            clip->setString("TEXT", [text UTF8String]);
        }
      }

      pxWindowNative::_helper_onKeyDown(mWindow, 86, 16);  // Fake a CTRL-V

      // Steal App Focus - become active App...
      [[NSRunningApplication currentApplication] activateWithOptions:(NSApplicationActivateAllWindows | NSApplicationActivateIgnoringOtherApps)];

      return YES;
    }

    return NO;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma mark - Dragging Operations (optionals)

-(void)draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint
{
}

- (void)draggingSession:(NSDraggingSession *)session movedToPoint:(NSPoint)screenPoint
{
}

- (void)draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
}

- (NSDragOperation)draggingSession:(NSDraggingSession *)session sourceOperationMaskForDraggingContext:(NSDraggingContext)context
{
    return NSDragOperationNone;
}

- (void)pasteboard:(NSPasteboard *)pasteboard item:(NSPasteboardItem *)item provideDataForType:(NSString *)type
{    
}

@end


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef PX_OPENGL
@interface MyView: NSOpenGLView
@end
#else
@interface MyView: NSView
@end
#endif

@implementation MyView
{
  pxWindow* mWindow;
  NSTrackingArea* trackingArea;
}

-(id)initWithPXWindow:(pxWindow*)window
{
  self = [super init];
  mWindow = window;
  //[self updateTrackingAreas];
  return self;
}

-(BOOL)isFlipped {
  return YES;
}

#ifdef PX_OPENGL
-(void)prepareOpenGL
{
  pxWindowNative::_helper_onCreate(mWindow);
}
#endif

-(void)mouseDown:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"mouseDown: %f, %f", p.x, p.y);
  pxWindowNative::_helper_onMouseDown(mWindow, p.x, p.y, PX_LEFTBUTTON);
}

-(void)mouseUp:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"mouseUp: %f, %f", p.x, p.y);
  pxWindowNative::_helper_onMouseUp(mWindow, p.x, p.y, PX_LEFTBUTTON);
}

-(void)rightMouseDown:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"rightMouseDown: %f, %f", p.x, p.y);
  pxWindowNative::_helper_onMouseDown(mWindow, p.x, p.y, PX_RIGHTBUTTON);
}

-(void)rightMouseUp:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"rightMouseUp: %f, %f", p.x, p.y);
  pxWindowNative::_helper_onMouseUp(mWindow, p.x, p.y, PX_RIGHTBUTTON);
}

-(void)mouseMoved:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
//  NSLog(@"mouseMoved: %f, %f", p.x, p.y);
  pxWindowNative::_helper_onMouseMove(mWindow, p.x, p.y);
}

-(void)keyDown:(NSEvent*)event
{
  //NSLog(@"keyDown, repeat:%s", event.ARepeat?"YES":"NO");
  if (!event.ARepeat)
  {
    // send px key down
    uint32_t flags = 0;
    
    if (event.modifierFlags & NSShiftKeyMask)     flags |= PX_MOD_SHIFT;
    if (event.modifierFlags & NSControlKeyMask)   flags |= PX_MOD_CONTROL;
    if (event.modifierFlags & NSAlternateKeyMask) flags |= PX_MOD_ALT;
    if (event.modifierFlags & NSCommandKeyMask)   flags |= PX_MOD_COMMAND;
    
    pxWindowNative::_helper_onKeyDown(mWindow, keycodeFromNative(event.keyCode), flags);
  }

  NSString* s = event.characters;
  if (![s isEqualToString:@""])
  {
    uint32_t c = [s characterAtIndex:0];
    // filter out control characters
    if (!iscntrl(c))
      pxWindowNative::_helper_onChar(mWindow, c);
  }
}

-(void)keyUp:(NSEvent*)event
{
  uint32_t flags = 0;
   
  if (event.modifierFlags & NSShiftKeyMask)     flags |= PX_MOD_SHIFT;
  if (event.modifierFlags & NSControlKeyMask)   flags |= PX_MOD_CONTROL;
  if (event.modifierFlags & NSAlternateKeyMask) flags |= PX_MOD_ALT;
  if (event.modifierFlags & NSCommandKeyMask)   flags |= PX_MOD_COMMAND;
  
  pxWindow::_helper_onKeyUp(mWindow,keycodeFromNative(event.keyCode), flags);
}

-(void)mouseEntered:(NSEvent*)event
{
  NSLog(@"mouseEntered");
  pxWindowNative::_helper_onMouseEnter(mWindow);
}

-(void)mouseExited:(NSEvent*)event
{
  NSLog(@"mouseExited");
  pxWindowNative::_helper_onMouseLeave(mWindow);
}

-(void)mouseDragged:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"mouseDragged: %f, %f", p.x, p.y);
  // send as px mousemove
  pxWindowNative::_helper_onMouseMove(mWindow, p.x, p.y);
}

-(void)rightMouseDragged:(NSEvent*)event
{
  NSPoint p = [event locationInWindow];
  p = [self convertPoint:p fromView:nil];
  NSLog(@"rightMouseDragged: %f, %f", p.x, p.y);
  // send as px mousemove
  pxWindowNative::_helper_onMouseMove(mWindow, p.x, p.y);
}

-(void)fireAnimationTimer:(NSTimer*)timer
{
  pxWindowNative::_helper_onAnimationTimer(mWindow);
}

-(void)updateTrackingAreas
{
  if(trackingArea != nil)
  {
    [self removeTrackingArea:trackingArea];
    [trackingArea release];
  }
  
  int opts = (NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInActiveApp |NSTrackingEnabledDuringMouseDrag);
  trackingArea = [ [NSTrackingArea alloc] initWithRect: [self bounds]
                                               options: opts
                                                 owner: self
                                              userInfo: nil];
  [self addTrackingArea:trackingArea];
}


-(void)drawRect:(NSRect)dirtyRect {
    pxWindowNative::_helper_onDraw(mWindow, (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort]);
#ifdef PX_OPENGL
  glFlush();
#endif
}

@end

#endif

// pxWindow

pxError pxWindow::init(int left, int top, int width, int height)
{
  pxWindowNative::registerWindow(this);
  NSPoint pos;
    
  NSRect  frame = [[NSScreen mainScreen] frame];
    
  pos.x = frame.origin.x + frame.size.width - width;
  pos.y = frame.origin.y + (frame.size.height-[[[NSApplication sharedApplication] mainMenu  ]menuBarHeight]) - height;

  NSWindow* window = [[NSWindow alloc] initWithContentRect: NSMakeRect(left, pos.y-top, width, height)
                                                 styleMask: NSTitledWindowMask | NSClosableWindowMask |NSMiniaturizableWindowMask | NSResizableWindowMask
                                                   backing: NSBackingStoreBuffered
                                                     defer: NO];
  
  NSWindowCollectionBehavior behavior = [window collectionBehavior];
  [window setCollectionBehavior: behavior | NSWindowCollectionBehaviorFullScreenPrimary ];
  
  mWindow = (void*)window;
  
  WinDelegate* delegate = [[WinDelegate alloc] initWithPXWindow:this];
  [window setDelegate:delegate];
   //[delegate release];  //<< TODO: Crashes if menus used.  Why ?
  mDelegate = (void*)delegate;
  MyView* view = [[MyView alloc] initWithPXWindow:this];
  
  [window setContentView: view];
  [window makeFirstResponder:view];
  
  [view release];

  [NSApp activateIgnoringOtherApps:YES];
  // TODO We get a GL crash if we don't show the window here... 
  //[window makeKeyAndOrderFront:NSApp];
  //[window orderOut: NSApp];
  
  onSize(width,height);
  onCreate();
  
  return PX_OK;
}

pxError pxWindow::term()
{
  NSWindow* window = (NSWindow*)mWindow;
  WinDelegate* delegate = (WinDelegate*)mDelegate;
  [window setDelegate:nil];
  [delegate release];
  mDelegate = nil;
  [window close];
  return PX_OK;
}

void pxWindow::invalidateRect(pxRect* pxr)
{
  NSWindow* window = (NSWindow*)mWindow;
  NSView* view = [window contentView];
  if (pxr)
  {
    NSRect r = NSMakeRect(pxr->left(), pxr->top(), pxr->width(), pxr->height());
    [view setNeedsDisplayInRect:r];
  }
  else
  {
    [view setNeedsDisplay:YES];
  }
  
  // needed to "forcibly allow" a drawRect call to occur whilst resizing a window
  if ([view inLiveResize])
  {
    [[NSOperationQueue mainQueue] addOperationWithBlock:^{
      [view displayIfNeeded];
    }];
  }
}

bool pxWindow::visibility()
{
  NSWindow *window = (NSWindow*)mWindow;
  return [window isVisible];
}

void pxWindow::setVisibility(bool visible)
{
  NSWindow* window = (NSWindow*)mWindow;
  if (visible)
    [window makeKeyAndOrderFront:NSApp];
  else
    [window orderOut:nil];
}

pxError pxWindow::setAnimationFPS(uint32_t fps)
{
  NSWindow* window = (NSWindow*)mWindow;
  
  if (mTimer)
  {
    NSTimer* t = (NSTimer*)mTimer;
    [t invalidate];
    mTimer = NULL;
  }
  
  if (fps > 0)
  {
    NSTimer* t = [NSTimer timerWithTimeInterval:1.0/fps target: [window contentView] selector: @selector(fireAnimationTimer:) userInfo:NULL repeats:YES];
    // Install it ourselves so that it runs during window resize etc... 
    [[NSRunLoop currentRunLoop] addTimer:t forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:t forMode:NSEventTrackingRunLoopMode];
    mTimer = (void*)t;
  }
  return PX_OK;
}

void pxWindow::setTitle(const char* title)
{
  NSWindow* window = (NSWindow*)mWindow;
  [window setTitle:[NSString stringWithUTF8String:title]];
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& s)
{
  // TODO
	return PX_OK;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& s)
{
  // TODO
	return PX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DEBUG CODE ... view (void*)  NSImage returned via XCode preview.
//
void* makeNSImage(void *rgba_buffer, int w, int h, int depth)
{
  size_t bitsPerComponent = 8;
  size_t bytesPerPixel    = depth; // RGBA
  size_t bytesPerRow      = w * bytesPerPixel;
  
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  
  if(!colorSpace)
  {
    NSLog(@"Error allocating color space RGB\n");
    return nil;
  }
  
  //Create bitmap context  
  CGContextRef context = CGBitmapContextCreate(rgba_buffer, w, h,
                                  bitsPerComponent,
                                  bytesPerRow,
                                  colorSpace,
                                  kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedLast);	// RGBA
  if(!context)
  {
    NSLog(@"Bitmap context not created");
  }
  
  CGImageRef  imageRef = CGBitmapContextCreateImage(context);
  NSImage*       image = [[NSImage alloc] initWithCGImage: imageRef
                                                     size: NSMakeSize(CGFloat(w),CGFloat(h))];
  
  // Flip Vertical ... Accommodate comparisons with OpenGL texutres
#if 0
  
 // CGContextRelease(context);  // might not be needed with ARC and/or Autorelease Pool
  CGColorSpaceRelease(colorSpace);
  
  return image;
  
#else
  
  NSAffineTransform       *transform = [NSAffineTransform transform];
  NSAffineTransformStruct       flip = {1.0, 0.0, 0.0, -1.0, 0.0, CGFloat(h) };
 
   NSImage *flipped = [[NSImage alloc] initWithSize: [image size]];
  
  [flipped lockFocus];
    [transform setTransformStruct: flip];
    [transform concat];
  
    [image drawAtPoint: NSMakePoint(0,0)
              fromRect: NSMakeRect(0,0, w, h)
             operation: NSCompositeCopy
              fraction: 1.0];
  
  [flipped unlockFocus];
 // [transform release];        // might not be needed with ARC and/or Autorelease Pool
  
 // CGContextRelease(context);  // might not be needed with ARC and/or Autorelease Pool
  CGColorSpaceRelease(colorSpace);
  
  return flipped;
#endif
}


// pxWindowNative
