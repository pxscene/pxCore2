
//
// g++ window.mm -framework Cocoa
//

#include <Cocoa/Cocoa.h>
#include <Foundation/Foundation.h>

@interface AppDelegate: NSObject<NSApplicationDelegate, NSFileManagerDelegate>
{
  NSWindow* mMainWindow;
}
-(id) initWithWindow: (NSWindow *) mainWindow;
@end

@implementation AppDelegate

-(id) initWithWindow: (NSWindow *) mainWindow
{
  self = [super init];
  mMainWindow = [mainWindow retain];
  return self;
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
  NSLog(@"applicationShouldTerminateAfterLastWindowClosed");
  return TRUE;
}

-(void)applicationWillTerminate:(NSNotification *)aNotification 
{
  NSLog(@"applicationWillTerminate");
}

-(void) dealloc
{
  [ super dealloc ];
  [ mMainWindow release ];
}

@end


int main(int argc, char* argv[])
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  [NSApplication sharedApplication];

  NSRect frame = NSMakeRect(0, 0, 640, 480);
  NSUInteger mask = 
    NSTitledWindowMask | 
    NSClosableWindowMask |
    NSResizableWindowMask |
    NSMiniaturizableWindowMask;

  NSWindow* window  = [[NSWindow alloc] initWithContentRect: frame
    styleMask: mask
    backing: NSBackingStoreBuffered
    defer: NO];

  AppDelegate* appDelegate = [[AppDelegate alloc] initWithWindow: window ];
 
  [window setBackgroundColor: [NSColor blueColor]];
  [window setShowsToolbarButton: TRUE ];
  [window makeKeyAndOrderFront: NSApp];


  [NSApp setDelegate: appDelegate];
  [NSApp run];
  
  [appDelegate release];
  [pool release];

  return 0;
}

