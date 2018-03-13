//  pxClipboardNative.cpp

#include <stdio.h>

#include "pxClipboardNative.h"

pxClipboardNative *pxClipboardNative::s_instance;


std::string pxClipboardNative::getString(std::string type)
{
    printf("pxClipboardNative::getString() - ENTER\n");
    
    // NSPasteboard*  pasteBoard = [NSPasteboard generalPasteboard];
    // NSString*        myString = [pasteBoard  stringForType: NSPasteboardTypeString];
    
    return std::string("(empty)");
}

void pxClipboardNative::setString(std::string type, std::string clip)
{
    printf("pxClipboardNative::setString() - ENTER\n");

    // NSString *stringToWrite = [NSString stringWithUTF8String: clip.c_str()];
    
    // NSPasteboard*  pasteBoard = [NSPasteboard generalPasteboard];

    // [pasteBoard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];
    // [pasteBoard setString:stringToWrite forType:NSStringPboardType];
}
