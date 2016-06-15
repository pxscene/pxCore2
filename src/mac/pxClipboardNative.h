//
//  pxClipboardNative.h
//  pxScene
//

#ifndef PX_CLIPBOARDNATIVE_H
#define PX_CLIPBOARDNATIVE_H

#import <string>

class pxClipboardNative
{
public:
             pxClipboardNative()  {};
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

#endif // PX_CLIPBOARDNATIVE_H
