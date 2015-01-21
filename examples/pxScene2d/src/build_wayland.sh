#WAYLAND=`pkg-config wayland-client wayland-egl --cflags --libs`

gcc -o utf8.o -c utf8.c

g++ -o gl2d pxMain.cpp pxScene2d.cpp pxText.cpp pxImage.cpp rtLog.cpp rtPathUtils.cpp rtString.cpp utf8.o pxUtil.cpp -I ../external/png -I ../../../src -I ../external/ft/include -L ../external/png/.libs/ -L ../external/ft/objs/.libs -D PX_PLATFORM_WAYLAND_EGL -l png16 -L ../../../build/wayland_egl -l pxCore -l rt -l freetype -lEGL -lGLESv2 `pkg-config wayland-client wayland-egl --cflags --libs`

