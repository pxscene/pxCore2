// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxEventLoopNative.cpp

#include "pxEventLoop.h"

void pxEventLoop::run()
{
    RunApplicationEventLoop();
}

void pxEventLoop::exit()
{
    QuitApplicationEventLoop();
}

int main(int argc, char* argv[])
{
    pxMain();
    return 0;
}
