//
//  pxClipboardNative.cpp
//  pxScene
//

#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "pxClipboardNative.h"

pxClipboardNative *pxClipboardNative::s_instance;




pxClipboardNative::pxClipboardNative()
{
 //   Display *dpy = XOpenDisplay(NULL);
    
     // Own clipboards
 //   XSetSelectionOwner(dpy, XA_PRIMARY, w, CurrentTime);

  //  Atom clipboard = XInternAtom(dpy, "CLIPBOARD", 0);
 //   XSetSelectionOwner(dpy, clipboard, w, CurrentTime);
}


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






















#if 0
extern Window juce_messageWindowHandle;
static String localClipboardContent;
static Atom atom_UTF8_STRING;
static Atom atom_CLIPBOARD;
static Atom atom_TARGETS;

//--------------------------------------------------------------------------------------------------------
static void initSelectionAtoms()
{
    static bool isInit = false;
    if (!isInit)
    {
        atom_UTF8_STRING = XInternAtom(display, "UTF8_STRING", False);
        atom_CLIPBOARD   = XInternAtom(display, "CLIPBOARD", False);
        atom_TARGETS   = XInternAtom(display, "TARGETS", False);
    }
}
//--------------------------------------------------------------------------------------------------------

/* read the content of a window property as either a locale-dependent string or an utf8 string 
   works only for strings shorter than 1000000 bytes 
*/
static String juce_x11_readWindowProperty(Window window, Atom prop,
        Atom fmt /* XA_STRING or UTF8_STRING */ ,
        bool deleteAfterReading)
{
        String returnData;
        uint8 * clipData;
        Atom actualType;
        int actualFormat;
        unsigned long nitems, bytesLeft;
        
        if (XGetWindowProperty(display, window, prop,
                0 L /* offset */ , 1000000 /* length (max) */ , False,
                AnyPropertyType /* format */ , & actualType, & actualFormat, & nitems, & bytesLeft, & clipData) == Success)
        {
            if (actualType == atom_UTF8_STRING && actualFormat == 8)
            {
                returnData = String::fromUTF8(clipData, nitems);
            }
            else if (actualType == XA_STRING && actualFormat == 8)
            {
                returnData = String((const char * ) clipData, nitems);
            }
            if (clipData != 0)
                XFree(clipData);
            jassert(bytesLeft == 0 || nitems == 1000000);
        }
        if (deleteAfterReading)
        {
            XDeleteProperty(display, window, prop);
        }
        return returnData;
    }
//--------------------------------------------------------------------------------------------------------

/* send a SelectionRequest to the window owning the selection and waits for its answer (with a timeout) */
static bool juce_x11_requestSelectionContent(String & selection_content, Atom selection, Atom requested_format)
{
    Atom property_name = XInternAtom(display, "JUCE_SEL", false);
    
    /* the selection owner will be asked to set the JUCE_SEL property on the juce_messageWindowHandle with the selection content */
    XConvertSelection(display, selection, requested_format, property_name, juce_messageWindowHandle, CurrentTime);
   
    bool gotReply = false;
    int timeoutMs = 200; // will wait at most for 200 ms
    do
    {
        XEvent event;
        gotReply = XCheckTypedWindowEvent(display, juce_messageWindowHandle, SelectionNotify, & event);
        if (gotReply)
        {
            if (event.xselection.property == property_name)
            {
                jassert(event.xselection.requestor == juce_messageWindowHandle); // or I didn't understand anything
                selection_content = juce_x11_readWindowProperty(event.xselection.requestor,
                    event.xselection.property, requested_format, true);
                return true;
            }
            else
            {
                return false; // the format we asked for was denied.. (event.xselection.property == None)
            }
        }
        /* not very elegant.. we could do a select() or something like that... however clipboard content requesting
           is inherently slow on x11, it often takes 50ms or more so... */
        Thread::sleep(4);
        timeoutMs -= 4;
    } while (timeoutMs > 0);
    
    DBG("timeout for juce_x11_requestSelectionContent");
    return false;
}

//--------------------------------------------------------------------------------------------------------

/* called from the event loop in juce_linux_Messaging in response to SelectionRequest events */
void juce_x11_handleSelectionRequest(XSelectionRequestEvent & evt)
{
    initSelectionAtoms();
    /* the selection content is sent to the target window as a window property */
    XSelectionEvent reply;
    reply.type = SelectionNotify;
    reply.display = evt.display;
    reply.requestor = evt.requestor;
    reply.selection = evt.selection;
    reply.target = evt.target;
    reply.property = None; // == "fail"
    reply.time = evt.time;

    char * data = 0;
    int property_format = 0, data_nitems = 0;
    if (evt.selection == XA_PRIMARY || evt.selection == atom_CLIPBOARD)
    {
        if (evt.target == XA_STRING)
        {
            // format data according to system locale
            data = strdup((const char * ) localClipboardContent);
            data_nitems = strlen(data);
            property_format = 8; // bits/item
        }
        else if (evt.target == atom_UTF8_STRING)
        {
            // translate to utf8
            data = strdup((const char * ) localClipboardContent.toUTF8());
            data_nitems = strlen(data);
            property_format = 8; // bits/item
        }
        else if (evt.target == atom_TARGETS)
        {
            // another application wants to know what we are able to send
            data_nitems = 2;
            property_format = 32; // atoms are 32-bit        
            data = (char * ) malloc(data_nitems * 4);
            ((Atom * ) data)[0] = atom_UTF8_STRING;
            ((Atom * ) data)[1] = XA_STRING;
        }
    }
    else
    {
        DBG("requested unsupported clipboard");
    }
    if (data)
    {
        const size_t MAX_REASONABLE_SELECTION_SIZE = 1000000;
        // for very big chunks of data, we should use the "INCR" protocol , which is a pain in the *ss
        if (evt.property != None && strlen(data) < MAX_REASONABLE_SELECTION_SIZE)
        {
            XChangeProperty(evt.display, evt.requestor,
                evt.property, evt.target,
                property_format /* 8 or 32 */ , PropModeReplace,
                (const unsigned char * ) data, data_nitems);
            reply.property = evt.property; // " == success"
        }
        free(data);
    }

    XSendEvent(evt.display, evt.requestor, 0, NoEventMask,
        (XEvent * ) & reply);
}

//--------------------------------------------------------------------------------------------------------

void SystemClipboard::copyTextToClipboard(const String & clipText) throw ()
{
    initSelectionAtoms();
    localClipboardContent = clipText;
    
    XSetSelectionOwner(display, XA_PRIMARY, juce_messageWindowHandle, CurrentTime);
    XSetSelectionOwner(display, atom_CLIPBOARD, juce_messageWindowHandle, CurrentTime);
}

//--------------------------------------------------------------------------------------------------------

const String SystemClipboard::getTextFromClipboard() throw ()
{
    String content; // the selection content
    initSelectionAtoms();
    /* 1) try to read from the "CLIPBOARD" selection first (the "high
   level" clipboard that is supposed to be filled by ctrl-C
   etc). When a clipboard manager is running, the content of this
   selection is preserved even when the original selection owner
   exits.

   2) and then try to read from "PRIMARY" selection (the "legacy" selection
   filled by good old x11 apps such as xterm)

   3) a third fallback could be CUT_BUFFER0 but they are obsolete since X10 !
   ( http://www.jwz.org/doc/x-cut-and-paste.html )

   There is not content negotiation here -- we just try to retrieve the selection first
   as utf8 and then as a locale-dependent string
*/
    Atom selection = XA_PRIMARY;
    Window selection_owner = None;
    if ((selection_owner = XGetSelectionOwner(display, selection)) == None)
    {
        selection = atom_CLIPBOARD;
        selection_owner = XGetSelectionOwner(display, selection);
    }

    if (selection_owner != None)
    {
        if (selection_owner == juce_messageWindowHandle)
        {
            content = localClipboardContent;
        } else
        {
            /* first try: we want an utf8 string */
            bool ok = juce_x11_requestSelectionContent(content, selection, atom_UTF8_STRING);
            if (!ok)
            {
                /* second chance, ask for a good old locale-dependent string ..*/
                ok = juce_x11_requestSelectionContent(content, selection, XA_STRING);
            }
        }
    }
    return content;
}

//--------------------------------------------------------------------------------------------------------
  //--------------------------------------------------------------------------------------------------------


#endif //00000