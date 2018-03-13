/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

//  pxClipboardNative.h

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
