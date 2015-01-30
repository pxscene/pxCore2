{
  "targets": [
    {
      "target_name": "px",
      "sources": [ 

        "jsCallback.cpp",
        "rtWrapperInit.cpp",
        "rtWrapperUtils.cpp",
        "rtFunctionWrapper.cpp",
        "rtObjectWrapper.cpp",
        "rtWindowWrapper.cpp",

        "../rtObject.cpp",
        "../rtLog.cpp",
        "../rtString.cpp",
        "../rtValue.cpp",
        "../utf8.c",

        "../pxContextGL.cpp",
        "../pxImage.cpp",
        "../pxImage9.cpp",
        "../pxScene2d.cpp",
        "../pxText.cpp",
        "../pxUtil.cpp"
       ],

      "include_dirs" : [
        "../",
        "../../external/png",
        "../../external/ft/include",
        "../../../../src"
      ],

      "libraries": [
        "-L../../../external/ft/objs/.libs/",
        "-L../../../external/png/.libs",
        "../../../../../build/x11/libpxCore.a",
        "-lX11",
        "-lfreetype",
        "-lglut",
        "-lGLEW",
        "-lpng16",

#        "-lGL",
#        "-lrt",
      ],

      "cflags": [
        "-DPX_PLATFORM_X11",
        "-DRT_PLATFORM_LINUX",
        "-DENABLE_GLUT",
        "-Wno-attributes"
      ]
    }
  ]
}
