{
  "targets": [
    {
      "target_name": "px",
      "sources": [ 
        "asynccontext.cpp",
        "eventloop.cpp",
        "init.cpp",
        "object.cpp",
        "offscreen.cpp",
        "scene2d.cpp",
        "window.cpp",
       ],
      "include_dirs" : [
        "../../../../src/",
        "../"
      ],
      "libraries": [
        "../../../../../build/x11/libpxCore.a",
        "-lX11"
      ]
    }
  ]
}
