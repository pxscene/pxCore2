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
        "../pxResource.cpp",
        "../pxConstants.cpp",
        "../rtThreadQueue.cpp",
        "../rtThreadPool.cpp",
        "../rtThreadTask.cpp",
        "../rtLibrary.cpp",
        "../rtZip.cpp",
        "../utf8.c",
        "../pxResource.cpp",
        "../pxConstants.cpp",
        "../pxContextGL.cpp",
        "../pxImage.cpp",
        "../pxImage9.cpp",
        "../pxScene2d.cpp",
        "../pxRectangle.cpp",
        "../pxFont.cpp",
        "../pxText.cpp",
        "../pxTextBox.cpp",
        "../pxWayland.cpp",
        "../pxUtil.cpp",
        "../pxInterpolators.cpp",
        "../pxFileDownloader.cpp",
        "../pxMatrix4T.cpp",
        "../pxTransform.cpp",
        "../pxArchive.cpp",
        "../unzip.c",
        "../ioapi.c",
        "../ioapi_mem.c",
       ],

      "include_dirs" : [
        "../",
        "../../external/png",
        "../../external/ft/include",
        "../../external/curl/include",
        "../../external/jpg",
        "../../external/zlib",
        "../../external/westeros/external/install/include",
        "../../../../src"
      ],

      "libraries": [
        "-L../../../external/ft/objs/.libs/",
        "-L../../../external/png/.libs",
        "-L../../../external/jpg/.libs",
        "-L../../../external/curl/lib/.libs/",
        "-L../../../external/westeros/external/install/lib",
        "../../../external/zlib/libz.a",
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
       "conditions": [
       ['"<!(ls ../../external | grep -s "westeros$" | cat)"=="westeros"',
          {
              'libraries':["-lwesteros_compositor"]          
          }
       ],
       ['"<!(ls ../../external | grep -s "westeros$" | cat)"!="westeros"',
          {
              'sources': ["../../external/westeros-stub/westeros-stub.cpp"],
              "include_dirs": ["../../external/westeros-stub"]           
          }
       ],
       ]
     }
    ],
    ['OS!="linux"',
       {
           'sources': ["../../external/westeros-stub/westeros-stub.cpp"],
           "include_dirs": ["../../external/westeros-stub"]           
       }
    ],

    ['OS=="linux"',
      {
        'sources': ["../linux/rtMutexNative.cpp", "../linux/rtThreadPoolNative.cpp"]
      },
    ],
    ['OS=="mac"',
      {
        'defines':['RUNINMAIN'],
        'sources': ["../linux/rtMutexNative.cpp", "../linux/rtThreadPoolNative.cpp"],
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
        'defines': [ "PX_PLATFORM_GLUT", "RT_PLATFORM_LINUX"]
      }
    ],
    ['OS=="win"',
      {
        'defines': ["PX_PLATFORM_GLUT", "RT_PLATFORM_WINDOWS"],
        'sources': ["../win/rtMutexNative.cpp", "../win/rtThreadPoolNative.cpp"]
      }
    ]
  ],

  'cflags_cc!': [ "-fno-rtti" ],

#      'cflags!': [
#        "-Wno-unused-parameter"
#      ],

      "cflags": [
         "-Wno-unused-parameter",
        "-Wno-attributes",
        "-Wall",
        "-Wextra"
      ]
    }
  ]
}
