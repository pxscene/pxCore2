TEMPLATE = app
TARGET = pxScene2d

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


DEFINES += ENABLE_DFB
DEFINES += PX_PLATFORM_X11
DEFINES += RT_PLATFORM_LINUX


SOURCES += \
    src/pxImage.cpp \
    src/pxImage9.cpp \
    src/pxImageDownloader.cpp \
    src/pxInterpolators.cpp \
    src/pxMain.cpp \
    src/pxMatrix4T.cpp \
    src/pxRectangle.cpp \
    src/pxScene2d.cpp \
    src/pxText.cpp \
    src/pxUtil.cpp \
    src/rtError.cpp \
    src/rtFile.cpp \
    src/rtLibrary.cpp \
    src/rtLog.cpp \
    src/rtObject.cpp \
    src/rtPathUtils.cpp \
    src/rtString.cpp \
    src/rtThreadPool.cpp \
    src/rtThreadTask.cpp \
    src/rtValue.cpp \
    src/testScene.cpp \
    src/utf8.c \
    src/linux/rtMutexNative.cpp \
    src/linux/rtThreadPoolNative.cpp

HEADERS += \
    src/pxContext.h \
    src/pxImage.h \
    src/pxImage9.h \
    src/pxImageDownloader.h \
    src/pxInterpolators.h \
    src/pxMatrix4T.h \
    src/pxRectangle.h \
    src/pxScene2d.h \
    src/pxText.h \
    src/pxTexture.h \
    src/pxUtil.h \
    src/rtAtomic.h \
    src/rtConfig.h \
    src/rtCore.h \
    src/rtDefs.h \
    src/rtError.h \
    src/rtFile.h \
    src/rtLibrary.h \
    src/rtLog.h \
    src/rtMutex.h \
    src/rtObject.h \
    src/rtObjectMacros.h \
    src/rtPathUtils.h \
    src/rtRefT.h \
    src/rtString.h \
    src/rtThreadPool.h \
    src/rtThreadTask.h \
    src/rtValue.h \
    src/testScene.h \
    src/utf8.h \
    src/linux/rtConfigNative.h \
    src/linux/rtMutexNative.h \
    src/linux/rtThreadPoolNative.h


contains(DEFINES, ENABLE_GLUT) {

message("Support 'pxScene2d' for GLUT ...")

SOURCES += \
    src/pxContextDescGL.cpp

HEADERS += \
    src/pxContextDescGL.h \
}


contains(DEFINES, ENABLE_DFB) {

message("Support 'pxScene2d' for DFB ...")

SOURCES += \
    src/pxContextDFB.cpp

HEADERS += \
    src/pxContextDescDFB.h
}


unix {
    target.path = /usr/lib
    INSTALLS += target

    INCLUDEPATH += src
    INCLUDEPATH += ../../src
    INCLUDEPATH += ./external/jpg
    INCLUDEPATH += ./external/curl/include
    INCLUDEPATH += ./external/ft/include
    INCLUDEPATH += ./external/png
    INCLUDEPATH += /usr/local/include/directfb

    LIBS += -L./external/jpg/.libs -ljpeg
    LIBS += -L../pxCore/examples/pxScene2d/external/png/.libs -lpng16
    LIBS += -L./external/ft/objs/.libs -lfreetype
    LIBS += -L./external/curl/lib/.libs -lcurl

    LIBS += -L/usr/local/lib -ldirectfb
   # LIBS += -L../pxCore/build/x11 -lpxCore
}


unix:!macx: LIBS += -L$$PWD/../../../build-pxCore-Desktop_Qt_5_2_1_GCC_64bit-Debug/ -lpxCore

INCLUDEPATH += $$PWD/../../../build-pxCore-Desktop_Qt_5_2_1_GCC_64bit-Debug
DEPENDPATH += $$PWD/../../../build-pxCore-Desktop_Qt_5_2_1_GCC_64bit-Debug
