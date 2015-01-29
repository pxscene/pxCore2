{
  "targets": [
    {
      "target_name": "px",
      "sources": [ 
        "asynccontext.cpp",
        "eventloop.cpp",
        "function.cpp",
        "init.cpp",
        "object.cpp",
        "offscreen.cpp",
        "scene2d.cpp",
        "window.cpp",

        "../rtObject.cpp",
        "../rtLog.cpp",
        "../rtString.cpp",
        "../rtValue.cpp",

        # scene2d sources
        "../pxContextGL.cpp",
        "../pxScene2d.cpp",
        "../pxImage.cpp"
       ],
      "include_dirs" : [
        "../../../../src/",
        "../"
      ],
      "libraries": [
#        "-L../../../external/ft/objs/.libs/",
#        "-L../../../external/png/.libs",
        "../../../../../build/x11/libpxCore.a",
        "-lX11",
#        "-lGL",
        "-lglut",
        "-lGLEW",
#        "-lrt",
#        "-lpng16",
#        "-lrt",
#        "-lfreetype"
      ],
      "cflags": [
        "-DPX_PLATFORM_X11",
        "-DENABLE_GLUT",
        "-Wno-attributes"
      ]
    }
  ]
}
