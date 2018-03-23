//
//  pxClipboardNative.cpp
//  pxScene
//

#import <Cocoa/Cocoa.h>

#include "pxClipboardNative.h"

pxClipboardNative *pxClipboardNative::s_instance;


std::string pxClipboardNative::getString(std::string type)
{
//    printf("pxClipboardNative::getString() - ENTER\n");
    
    NSPasteboard*  pasteBoard = [NSPasteboard generalPasteboard];
    NSString*        myString = [pasteBoard  stringForType: NSPasteboardTypeString];
  
  if(myString)
  {
    return std::string([myString UTF8String]);
  }
  else
  {
    return std::string("");
  }
}

void pxClipboardNative::setString(std::string type, std::string clip)
{
//    printf("pxClipboardNative::setString() - ENTER\n");

    NSString *stringToWrite = [NSString stringWithUTF8String: clip.c_str()];
  
  if(stringToWrite)
  {
    NSPasteboard*  pasteBoard = [NSPasteboard generalPasteboard];

    if(pasteBoard)
    {
      [pasteBoard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
      [pasteBoard setString:stringToWrite forType:NSStringPboardType];
    }
  }
}
