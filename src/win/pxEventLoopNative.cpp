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

// pxEventLoopNative.cpp

#define PX_NATIVE
#include "../pxEventLoop.h"

#include <windows.h>

void pxEventLoop::run()
{
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void pxEventLoop::exit()
{
    PostQuitMessage(0);
}


/////////////////////////////////////
// Windows specific entrypoint

// Used when /SUBSYSTEM:WINDOWS
int CALLBACK WinMain(
  HINSTANCE hInstance,
  HINSTANCE hInstNULL,
#ifdef UNDER_CE
    LPWSTR lpCmdLine,
#else

    LPSTR lpCmdLine,
#endif
  int       nCmdShow)
{
	//extern int __argc;          /* count of cmd line args */
	//extern char ** __argv;      /* pointer to table of cmd line args */
	//pxMain(__argc,__argv);
	pxMain(0, NULL);
	return 0;
}

// Used when /SUBSYSTEM:CONSOLE
#ifdef UNICODE
int wmain(int argc, wchar_t** argv)
#else
int main(int argc, char** argv)
#endif
{
	for (int i = 0; i < argc; i++) {
		if (!strcmp(argv[i], "-hide-console")) {
			ShowWindow(GetConsoleWindow(), SW_HIDE);
		}
	}
  pxMain(argc,argv);
  return 0;
}
