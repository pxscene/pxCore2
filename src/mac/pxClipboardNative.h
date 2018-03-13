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
