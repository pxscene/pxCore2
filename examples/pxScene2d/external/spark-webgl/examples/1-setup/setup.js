var gles2 = require('../../gles2');

var options = {width: 1280, height: 720, title: "Setup"};
var gl = gles2.init(options);

var count = 0;
var ival = setInterval(function() {
    gl.clearColor(0, 1.0, 1.0, 0.0);

    // Set the viewport
    gl.viewport( 0, 0, 1280, 720);

    // Clear the color buffer
    gl.clear(gl.COLOR_BUFFER_BIT);

    gles2.nextFrame();

    if (count++ > 100) {
        clearInterval(ival);
    }
}, 20);
