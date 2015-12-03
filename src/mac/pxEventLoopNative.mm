// pxCore CopyRight 2005-2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxEventLoopNative.cpp

#include "pxEventLoop.h"

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#if 1
@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
  // Insert code here to tear down your application
}

@end
#endif


void pxEventLoop::run()
{
  NSArray* tl = NULL;
  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
  NSApplication* application = [NSApplication sharedApplication];
  [[NSBundle mainBundle] loadNibNamed:@"MainMenu" owner:application topLevelObjects:&tl];
  
  AppDelegate *appDelegate = [[AppDelegate alloc] init];
  [NSApp setDelegate:appDelegate];
  [NSApp run];
  [pool release];
}

bool init = false;

void pxEventLoop::runOnce()
{
  if (!init)
  {
    //NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    //NSApplication* application = [NSApplication sharedApplication];
    //[[NSBundle mainBundle] loadNibNamed:@"MainMenu" owner:application topLevelObjects:&tl];
  
    AppDelegate *appDelegate = [[AppDelegate alloc] init];
    [NSApp setDelegate:appDelegate];
    
#if 1
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    /*OSStatus returnCode = */TransformProcessType(& psn, kProcessTransformToForegroundApplication);
    
    [NSApp activateIgnoringOtherApps:YES];
#endif
    
  init = true;
  }

#if 0
  CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.02, 0);
#else
  [NSApplication sharedApplication]; // Make sure NSApp exists
  NSEvent* event = nil;
  
  while ((event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                     untilDate:[NSDate distantPast]
                                        inMode:NSDefaultRunLoopMode
                                       dequeue:YES])) // Remove the event from the queue
  {
    [NSApp sendEvent:event];
  }
  
#if 0
  while ((event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                     untilDate:[NSDate distantPast]
                                        inMode:NSEventTrackingRunLoopMode
                                       dequeue:YES])) // Remove the event from the queue
  {
    [NSApp sendEvent:event];
  }
#endif
#endif
}

void pxEventLoop::exit()
{
  [NSApp terminate:nil];
}

int main(int argc, char* argv[])
{
    pxMain();
    return 0;
}
