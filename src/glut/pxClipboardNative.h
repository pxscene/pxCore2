//
//  pxClipboardNative.h
//  pxScene
//

#ifndef PX_CLIPBOARDNATIVE_H
#define PX_CLIPBOARDNATIVE_H

#include <string>

class pxClipboardNative
{
public:
             pxClipboardNative();
    virtual ~pxClipboardNative()  {};
    
    
    std::string getString(std::string type);
           void setString(std::string type, std::string clip);
    
    static pxClipboardNative *instance()
    {
        if (!s_instance)
        {
            s_instance = new pxClipboardNative();
        }
        
        return s_instance;
    }
    
private:
    static pxClipboardNative *s_instance;
};


// /* An instance of a MULTIPLE SelectionRequest being served */
// typedef struct _MultTrack MultTrack;

// struct _MultTrack
// {
//   MultTrack *mparent;
//   Display   *display;
//   Window     requestor;
  
//   Atom       property;
//   Atom       selection;
  
//   Time       time;
//   Atom      *atoms;
  
//   unsigned long length;
//   unsigned long index;
//   unsigned char * sel;
// };

/* Selection serving states */
typedef enum
{
  S_NULL=0,
  S_INCR_1,
  S_INCR_2
}
IncrState;

/* Status of request handling */
typedef int HandleResult;

#define HANDLE_OK         0
#define HANDLE_ERR        (1<<0)
#define HANDLE_INCOMPLETE (1<<1)
#define DID_DELETE        (1<<2)

#endif // PX_CLIPBOARDNATIVE_H
