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
        "../utf8.c",
        "../pxContextGL.cpp",
        "../pxImage.cpp",
        "../pxImage9.cpp",
        "../pxScene2d.cpp",
        "../pxRectangle.cpp",
        "../pxText.cpp",
        "../pxText2.cpp",
        "../pxUtil.cpp",
        "../pxInterpolators.cpp",
        "../pxFileDownloader.cpp",
        "../pxTextureCacheObject.cpp",
        "../pxTexture.cpp",
        "../pxMatrix4T.cpp",
        "../pxTransform.cpp",
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
        "-lfreetype",
        "-lpng16",
        "-lcurl",
        "-ldl"
#        "-lrt",
      ],

  "conditions": [
    ['OS=="linux"',
      {
        'sources': ["../linux/rtMutexNative.cpp", "../linux/rtThreadPoolNative.cpp"]
      },
    ],
    ['OS=="mac"',
      {
        'defines':['RUNINMAIN'],
        'libraries': ["-framework GLUT", "-framework OpenGL", "../../../external/jpg/.libs/libjpeg.a"]
      }
    ],
    ['OS!="mac"',
      {
        'libraries': ["-lglut", "-lGL", "-lGLEW", "-ljpeg"]
      }
    ],
    ['OS!="win"',
      {
        'defines': [ "PX_PLATFORM_GLUT", "RT_PLATFORM_LINUX", "RT_USE_SINGLE_RENDER_THREAD"]
      }
    ],
    ['OS=="win"',
      {
        'defines': ["PX_PLATFORM_GLUT", "RT_PLATFORM_WINDOWS"],
        'sources': ["../win/rtMutexNative.cpp", "../win/rtThreadPoolNative.cpp"]
      }
    ]
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
