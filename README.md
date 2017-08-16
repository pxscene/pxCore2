#pxCore
Updated July 19th 2009

pxCore is a small opensource library that provides a portable framebuffer and windowing abstraction for C++. This library is intended to hide the complexity of writing rasterization code on multiple platforms and allows you to easily write the following sorts of things in a portable way.

* 2d and 3d rasterizers
* Transition Libraries
* Filter Routines
* Image Processing

In its design a few principles were followed:

* Be Small – A simple windowed application can be built (on Windows) in as little as 8k.
* Be Simple – The framebuffer abstraction supports 32bpp framebuffers and is intended to be minimal yet complete.
* Don’t tightly couple the framebuffer and windowing functionality. Some other framebuffer libraries (PixelToaster for one) don’t separate out the framebuffer abstraction from the windowing abstraction. By loosely coupling the two abstractions this library becomes much more valuable; as the framebuffer functionality can be used and integrated with other windowing toolkits easily thereby making YOUR code more reusable.
* Platform native surface construction
* Policy free resizing support – No policy is baked into the window resizing support so that applications completely control their own resizing behavior.
* Portable Keyboard, Mouse and Window events
* Support for portable performance timers
* Basic Animation Support - Support for a basic animation timer event (frames per second) is built into the windowing abstraction making it easy to write applications that animate their contents.
* pxCore has been ported to Windows, pocketpc (arm4), linux(x11), OSX.

Migrated from original (Google code)[https://code.google.com/p/pxcore/] repo December 14, 2014
Google Code is going away. (Original Code Copied To Github)[https://github.com/johnrobinsn/pxcore.old]


