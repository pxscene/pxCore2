#-------------------------------------------------
#
# Project created by QtCreator 2015-02-10T07:36:37
#
#-------------------------------------------------


QT       -= gui

TARGET = pxCore
TEMPLATE = lib

DEFINES += PXCORE_LIBRARY
DEFINES += ENABLE_DFB
DEFINES += PX_PLATFORM_X11

SOURCES += \
    src/pxOffscreen.cpp \
    src/pxViewWindow.cpp \
    src/pxWindowUtil.cpp

HEADERS +=\
        pxcore_global.h \
    src/pxBuffer.h \
    src/pxColors.h \
    src/pxConfig.h \
    src/pxCore.h \
    src/pxEventLoop.h \
    src/pxIView.h \
    src/pxKeycodes.h \
    src/pxOffscreen.h \
    src/pxPixels.h \
    src/pxRect.h \
    src/pxTimer.h \
    src/pxViewWindow.h \
    src/pxWindow.h \
    src/pxWindowUtil.h \
    src/rtRefPtr.h




contains(DEFINES,ENABLE_GLUT) {

message("Support 'pxCore' for GLUT ...")

SOURCES += \
    src/x11/pxWindowNativeGlut.cpp

HEADERS += \
    src/x11/pxWindowNativeGlut.h \
}


contains(DEFINES,ENABLE_DFB) {

message("Support 'pxCore' for DFB ...")

INCLUDEPATH += /usr/local/include/directfb

SOURCES += \
    src/x11/pxBufferNativeDfb.cpp \
    src/x11/pxWindowNativeDfb.cpp \
    src/x11/pxOffscreenNativeDfb.cpp

HEADERS += \
    src/x11/pxBufferNativeDfb.h \
    src/x11/pxWindowNativeDfb.h \
    src/x11/pxOffscreenNativeDfb.h
}


contains(DEFINES,PX_PLATFORM_X11) {
SOURCES += \
#    src/x11/pxBufferNative.cpp \
#    src/x11/pxEventLoopNative.cpp \
#    src/x11/pxOffscreenNative.cpp \
#    src/x11/pxTimerNative.cpp \
#    src/x11/pxWindowNative.cpp

HEADERS += \
#    src/x11/pxBufferNative.h \
#    src/x11/pxConfigNative.h
#    src/x11/pxOffscreenNative.h
#    src/x11/pxWindowNative.h
}



contains(DEFINES,PX_PLATFORM_WIN) {
SOURCES += \
    src/win/pxBufferNative.cpp

#HEADERS += \

}


contains(DEFINES,PX_PLATFORM_MAC) {
SOURCES += \
    src/mac/pxBufferNative.cpp \
    src/mac/pxEventLoopNative.cpp \
    src/mac/pxOffscreenNative.cpp \
    src/mac/pxTimerNative.cpp \
    src/mac/pxWindowNative.cpp

HEADERS += \
    src/mac/pxConfigNative.h \
    src/mac/pxBufferNative.h \
    src/mac/pxOffscreenNative.h \
    src/mac/pxWindowNative.h
}



contains(DEFINES,PX_PLATFORM_WAYLAND) {
SOURCES += \
    src/wayland/pxBufferNative.cpp \
    src/wayland/pxEventLoopNative.cpp \
    src/wayland/pxOffscreenNative.cpp \
    src/wayland/pxTimerNative.cpp \
    src/wayland/pxWindowNative.cpp \

HEADERS += \
    src/wayland/pxConfigNative.h \
    src/wayland/pxBufferNative.h \
    src/wayland/pxOffscreenNative.h \
    src/wayland/pxWindowNative.h
}


contains(DEFINES,PX_PLATFORM_WAYLAND_EGL) {
SOURCES += \
    src/wayland_egl/pxBufferNative.cpp \
    src/wayland_egl/pxEventLoopNative.cpp \
    src/wayland_egl/pxOffscreenNative.cpp \
    src/wayland_egl/pxTimerNative.cpp \
    src/wayland_egl/pxWindowNative.cpp

HEADERS += \
    src/wayland_egl/pxConfigNative.h \
    src/wayland_egl/pxBufferNative.h \
    src/wayland_egl/pxOffscreenNative.h \
    src/wayland_egl/pxWindowNative.h
}


unix {
    target.path = /usr/lib
    INSTALLS += target

}

OBJECTIVE_SOURCES += \
    src/mac/window.mm
