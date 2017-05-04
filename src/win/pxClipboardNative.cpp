//
//  pxClipboardNative.cpp
//  pxScene
//

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
