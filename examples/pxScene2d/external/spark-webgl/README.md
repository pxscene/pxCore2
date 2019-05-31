spark-webgl derived from wpe-webgl
=====

[![npm version](https://badge.fury.io/js/wpe-webgl.svg)](https://badge.fury.io/js/wpe-webgl)
[![npm](https://img.shields.io/npm/dt/wpe-webgl.svg)]()
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=55UJZHTXW8VTE)

Provides a WebGL rendering context for nodejs. It is displayed in a window or fullscreen, depending on your platform.

# Notes
Because on some targets OpenGL must be used instead of OpenGL ES2, you **must** do the following in your
shaders:

    #ifdef GL_ES
    precision highp float;
    #endif

# Installation

## Linux, OSX, Windows (GLEW/GLFW platforms)
GLFW and GLEW provides us with the functionality to start a OpenGL-enabled window.
There are minor differences between OpenGL ES2 (WebGL) and OpenGL using GLEW.
For more information, see: https://www.khronos.org/webgl/wiki_1_15/index.php/WebGL_and_OpenGL_Differences

## Raspberry PI
This module requires node 4+. Please install from nodesource 

    curl -sL https://deb.nodesource.com/setup_7.x | sudo -E bash -

Use Raspbian or make sure that includes and libs are in /opt/vc.
Raspbian has a default GPU memory setting of 64M, which is quite low. It may lead to 0x0505 (out of memory) errors.
You can increase this to a higher number using raspi-config.

### Dependencies
Linux: libglew-dev libglfw3-dev

Mac OSX: Use Homebrew
    brew install pkg-config glfw3 glew
For help with issues when installing pkg-config, see https://github.com/Automattic/node-canvas/wiki/Installation---OSX.

Windows: glew32.lib opengl32.lib

# Example
    var webgl = require('wpe-webgl');

    var options = {width: 1280, height: 720};
    var gl = webgl.init(options);

    while(true) {
        gl.clearColor(0, 1.0, 1.0, 0.0);

        // Set the viewport
        gl.viewport( 0, 0, 1280, 720);

        // Clear the color buffer
        gl.clear(gl.COLOR_BUFFER_BIT);

        // Do other GL-related stuff here.

        webgl.nextFrame(true /* Use false to prevent buffer swapping */);
    }

A couple of more elaborate examples can be found in the examples folder.

# Options
| Name          | Description            |
| ------------- |:----------------------:|
| width         | viewport width in px   |
| height        | viewport height in px  |
| fullscreen    | window or fullscreen?  |
| title         | window title           |
