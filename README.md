# pxCore

pxCore is a small open source library that provides a portable framebuffer and windowing abstraction for C++. This library is intended to hide the complexity of writing rasterization code on multiple platforms and allows you to easily write the following sorts of things in a portable way:

* 2d and 3d rasterizers
* Transition Libraries
* Filter Routines
* Image Processing

In its design, a few principles were followed:

* Be Small – A simple windowed application can be built (on Windows) in as little as 8k.
* Be Simple – The framebuffer abstraction supports 32bpp framebuffers and is intended to be minimal yet complete.
* Don’t tightly couple the framebuffer and windowing functionality. Some other framebuffer libraries (eg. PixelToaster) don’t separate out the framebuffer abstraction from the windowing abstraction. By loosely coupling the two abstractions, this library becomes much more valuable. The framebuffer functionality can be used and integrated with other windowing toolkits easily, making YOUR code more reusable.
* Platform native surface construction
* Policy free resizing support – No policy is baked into the window resizing support so that applications completely control their own resizing behavior.
* Portable Keyboard, Mouse and Window events
* Support for portable performance timers
* Basic Animation Support - Support for a basic animation timer event (frames per second) is built into the windowing abstraction, making it easy to write applications that animate their contents.
* pxCore has been ported to Windows, pocketpc (arm4), linux(x11), and OSX.


The most popular usage of pxCore is __pxscene__. __pxscene__ is an application engine that exposes a scene graph API to a Javascript engine. It gives JavaScript applications access to the pxscene API for visual elements that can be used for composition.  __pxscene__ is written on top of pxCore. 

__pxscene__: 

* supports rich animation and alpha masking primitives
* exposes a DOM-like programming model
* supports promises for asynchronous behaviors
* uses W3C event bubbling semantics

__pxscene__ source code is in pxCore/examples/pxScene2d.
Instructions for building __pxscene__ are here: [Building pxscene](https://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md)
Instructions for debugging __pxscene__ JavaScript applications using VSCode are here: [Debugging applications](https://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/VSCODE_DEBUG.md)


