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

        "../rtError.cpp",
        "../rtObject.cpp",
        "../rtLog.cpp",
        "../rtString.cpp",
        "../rtValue.cpp",
        "../rtFile.cpp",
        "../rtThreadPool.cpp",
        "../rtThreadTask.cpp",
        "../rtLibrary.cpp",
        "../linux/rtMutexNative.cpp",
        "../linux/rtThreadPoolNative.cpp",
        "../utf8.c",

        "../pxContextGL.cpp",
        "../pxImage.cpp",
        "../pxImage9.cpp",
        "../pxScene2d.cpp",
        "../pxRectangle.cpp",
        "../pxText.cpp",
        "../pxUtil.cpp",
        "../pxInterpolators.cpp",
        "../pxFileDownloader.cpp",
        "../pxTextureCacheObject.cpp"
       ],

      "include_dirs" : [
        "../",
        "../../external/png",
        "../../external/ft/include",
        "../../external/curl/include",
        "../../external/jpg",
        "../../../../src"
      ],

      "libraries": [
        "-L../../../external/ft/objs/.libs/",
        "-L../../../external/png/.libs",
        "-L../../../external/jpg/.libs",
        "-L../../../external/curl/lib/.libs/",
        "../../../../../build/glut/libpxCore.a",
#        "-lX11",
        "-lfreetype",
        "-lglut",
        "-lGLEW",
        "-lpng16",
        "-ljpeg",
        "-lcurl",
        "-ldl"
#        "-lGL",
#        "-lrt",
      ],

      "defines": [
        "PX_PLATFORM_GLUT",
        "RT_PLATFORM_LINUX",
#        "ENABLE_GLUT",
      ],

      'cflags!': [
        "-Wno-unused-parameter"
      ],

      "cflags": [
        "-Wno-attributes",
        "-Wall",
        "-Wextra"
      ]
    }
  ]
}
