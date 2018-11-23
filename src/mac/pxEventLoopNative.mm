// pxCore CopyRight 2005-2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxEventLoopNative.cpp

#include "pxEventLoop.h"
#include <pxWindow.h>
#include "pxWindowNative.h"
#include <unistd.h>

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#if 1
@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@interface AppDelegate ()
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    NSLog(@"   applicationDidFinishLaunching()  MENU");
    
    NSMenu *bar = [[NSMenu alloc] init];
    [NSApp setMainMenu: bar];

    id appName = [[NSProcessInfo processInfo] processName];

    // APP ---------------------------------------------------------------------------------

    NSMenuItem *appMenuItem = [bar addItemWithTitle:@"Title" action:NULL keyEquivalent:@""];
    
    NSMenu *appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];

    
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
                       action:@selector(orderFrontStandardAboutPanel:)
                keyEquivalent:@""];
    
//        [appMenu addItem:[NSMenuItem separatorItem]]; // -----------
//        
//        NSMenu *servicesMenu = [[NSMenu alloc] init];
//        [NSApp setServicesMenu:servicesMenu];
//        [[appMenu addItemWithTitle:@"Services"
//                            action:NULL
//                     keyEquivalent:@""] setSubmenu:servicesMenu];
    
    [appMenu addItem:[NSMenuItem separatorItem]]; // -----------
    
    [appMenu addItemWithTitle: [NSString stringWithFormat:@"Hide %@", appName]
                       action: @selector(hide:)
                keyEquivalent: @"h"];
    
    [[appMenu addItemWithTitle: @"Hide Others"
                        action: @selector(hideOtherApplications:)
                 keyEquivalent: @"h"]
     
     setKeyEquivalentModifierMask: NSAlternateKeyMask | NSCommandKeyMask];
    
    [appMenu addItemWithTitle: @"Show All"
                       action: @selector(unhideAllApplications:)
                keyEquivalent: @""];
    
    [appMenu addItem:[NSMenuItem separatorItem]]; // -----------
    
    [appMenu addItemWithTitle: [NSString stringWithFormat:@"Quit %@", appName]
                       action: @selector(terminate:)
                keyEquivalent: @"q"];
  
    [appMenu release];
  
    // FILE ---------------------------------------------------------------------------------
    
    NSMenuItem *fileMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    
    NSMenu *fileMenu = [[NSMenu alloc] initWithTitle:@"File"];
    
    [NSApp          setWindowsMenu: fileMenu];
    [fileMenuItem setSubmenu: fileMenu];
    
    [fileMenu addItemWithTitle: @"New Window"
                        action: @selector(newWindow:)
                 keyEquivalent: @"n"];
    
    [fileMenu addItemWithTitle: @"Miniaturize"
                        action: @selector(performMiniaturize:)
                 keyEquivalent: @"m"];
    
    [fileMenu addItemWithTitle: @"Zoom"
                        action: @selector(performZoom:)
                 keyEquivalent: @""];
    
    [fileMenu addItem:[NSMenuItem separatorItem]]; // -----------
    
    [fileMenu addItemWithTitle: @"Bring All to Front"
                        action: @selector(arrangeInFront:)
                 keyEquivalent: @""];

    [fileMenu release];
  
    // EDIT ---------------------------------------------------------------------------------
    
    NSMenuItem *editMenuItem =
    [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    
    NSMenu *editMenu = [[NSMenu alloc] initWithTitle:@"Edit"];
    
    [NSApp    setWindowsMenu: editMenu];
    [editMenuItem setSubmenu: editMenu];
    
    [editMenu addItemWithTitle: @"Cut"
                        action: @selector(cut:)
                 keyEquivalent: @"x"];
    
    [editMenu addItemWithTitle: @"Copy"
                        action: @selector(copy:)
                 keyEquivalent: @"c"];
    
    [editMenu addItemWithTitle: @"Paste"
                        action: @selector(paste:)
                 keyEquivalent: @"v"];
    
    [editMenu addItem:[NSMenuItem separatorItem]]; // -----------
  
    [editMenu release];
  
    
    // VIEW ---------------------------------------------------------------------------------
    
    NSMenuItem *viewMenuItem =
    [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    
    NSMenu *viewMenu = [[NSMenu alloc] initWithTitle:@"View"];
    
    [NSApp    setWindowsMenu: viewMenu];
    [viewMenuItem setSubmenu: viewMenu];
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundeclared-selector"
  
    [[viewMenu addItemWithTitle: @"Toggle Address Bar"
                         action: @selector(toggleAddressBar:)
                  keyEquivalent: @"f"]
     setKeyEquivalentModifierMask:NSControlKeyMask | NSAlternateKeyMask];
  
#pragma clang diagnostic pop  

    [viewMenu addItem:[NSMenuItem separatorItem]]; // -----------

  
    [[viewMenu addItemWithTitle: @"Enter Full Screen"
                        action: @selector(toggleFullScreen:)
                 keyEquivalent: @"f"]
       setKeyEquivalentModifierMask:NSControlKeyMask | NSCommandKeyMask];
        
    [viewMenu release];
    
    // WINDOW ---------------------------------------------------------------------------------
    
    NSMenuItem *windowMenuItem =
    [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    
    NSMenu *windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    [NSApp          setWindowsMenu: windowMenu];
    [windowMenuItem setSubmenu: windowMenu];
    
    [windowMenu addItemWithTitle:@"Miniaturize"
                          action:@selector(performMiniaturize:)
                   keyEquivalent:@"m"];
    
    [windowMenu addItemWithTitle:@"Zoom"
                          action:@selector(performZoom:)
                   keyEquivalent:@""];
    
    [windowMenu addItem:[NSMenuItem separatorItem]]; // -----------
    
    [windowMenu addItemWithTitle:@"Bring All to Front"
                          action:@selector(arrangeInFront:)
                   keyEquivalent:@""];

    [windowMenu release];
  
    // ---------------------------------------------------------------------------------
  
    [bar release];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
  // Insert code here to tear down your application
  pxWindowNative::closeAllWindows();
  //sleep for few seconds to detect leak
  char const* s = getenv("ENABLE_MEMLEAK_CHECK");
  if (s && (strcmp(s,"1") == 0))
  {
    NSLog(@"   willTerminate  sleep so, valgrind can take memory report");
    sleep(30);
  }
}

@end
#endif


void pxEventLoop::run()
{
  NSArray* tl = NULL;
  NSAutoreleasePool    *pool = [[NSAutoreleasePool alloc] init];
  NSApplication *application = [NSApplication sharedApplication];
    
  [[NSBundle mainBundle] loadNibNamed:@"MainMenu" owner:application topLevelObjects:&tl];
  
  AppDelegate *appDelegate = [[AppDelegate alloc] init];
  [NSApp setDelegate:appDelegate];
  [NSApp run];
  
  [appDelegate release];
  
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
    
    [appDelegate release];

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
    pxMain(argc,argv);
    return 0;
}
