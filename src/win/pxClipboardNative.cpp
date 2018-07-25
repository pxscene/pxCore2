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

//  pxClipboardNative.cpp

#include <stdio.h>

#include "pxClipboardNative.h"

pxClipboardNative *pxClipboardNative::s_instance;
#include "windows.h"

std::string pxClipboardNative::getString(std::string type)
{
	std::string ret = "";
  printf("pxClipboardNative::getString() - ENTER\n");
	OpenClipboard(NULL);
	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		HANDLE h = GetClipboardData(CF_TEXT);  
		char* p = (char*)GlobalLock(h);
		ret = std::string(p);
		GlobalUnlock(h);
	}	
	CloseClipboard();
	return ret;
}

void pxClipboardNative::setString(std::string type, std::string clip)
{
    printf("pxClipboardNative::setString() - ENTER\n");

		OpenClipboard(NULL);  
		EmptyClipboard();  
		HANDLE hHandle = GlobalAlloc(GMEM_FIXED, clip.length() + 1);
		char* pData = (char*)GlobalLock(hHandle);  
		strcpy(pData, clip.c_str());
		SetClipboardData(CF_TEXT, hHandle);  
		GlobalUnlock(hHandle);
		CloseClipboard();  

}
