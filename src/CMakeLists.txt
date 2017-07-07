cmake_minimum_required(VERSION 2.8)
project(pxcore)

set(CMAKE_CXX_STANDARD 11)

option(WINDOWLESS_EGL "WINDOWLESS_EGL" OFF)
if(WIN32)
    option(PXCORE_COMPILE_WARNINGS_AS_ERRORS "PXCORE_COMPILE_WARNINGS_AS_ERRORS" OFF)
else()
    option(PXCORE_COMPILE_WARNINGS_AS_ERRORS "PXCORE_COMPILE_WARNINGS_AS_ERRORS" ON)
endif(WIN32)

set(EXTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../examples/pxScene2d/external")
set(NODEPATH "${EXTDIR}/libnode-v6.9.0/")
set(NODEINC "${NODEPATH}/src ${NODEPATH}/deps/uv/include ${NODEPATH}/deps/v8 ${NODEPATH}/deps/debugger-agent/include ${NODEPATH}/deps/v8/include ${NODEPATH}/deps/cares/include")
set(ZLIBINC "${EXTDIR}/zlib-1.2.8/")
set(CURLINC "${EXTDIR}/curl-7.40.0/include")
set(JPGINC "${EXTDIR}/jpeg-9a")
set(JPGTURBOINC "${EXTDIR}/libjpeg-turbo")
set(PNGINC "${EXTDIR}/libpng-1.6.28")
set(PTHREADINC "${EXTDIR}/pthread-2.9")
set(BUILD_RTCORE_SHARED_LIBRARY 1)

if (APPLE)
    message("Building pxcore for mac")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -fPIC -g")
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/mac)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/mac)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/mac)
    add_definitions(-DPX_PLATFORM_MAC -DENABLE_RT_NODE -DENABLE_HTTP_CACHE -DRUNINMAIN)
    add_definitions(-D_DARWIN_USE_64_BIT_INODE=1 -DNODE_ARCH="x64" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS= -DNODE_SHARED_MODE
            -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1
            -DV8_INSPECTOR_USE_OLD_STL=1 -DHAVE_OPENSSL=1 -DHAVE_DTRACE=1 -D__POSIX__ -DNODE_PLATFORM=darwin
            -DUCONFIG_NO_TRANSLITERATION=1 -DUCONFIG_NO_SERVICE=1 -DUCONFIG_NO_REGULAR_EXPRESSIONS=1 -DU_ENABLE_DYLOAD=0
            -DU_STATIC_IMPLEMENTATION=1 -DU_HAVE_STD_STRING=0 -DUCONFIG_NO_BREAK_ITERATION=0 -DUCONFIG_NO_LEGACY_CONVERSION=1
            -DUCONFIG_NO_CONVERSION=1 -DHTTP_PARSER_STRICT=0 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
            -DRT_PLATFORM_LINUX -DENABLE_NODE_V_6_9 -DENABLE_DEBUG_MODE -DPNG_APNG_SUPPORTED)
    set(PXCORE_FILES mac/pxBufferNative.mm mac/pxOffscreenNative.mm mac/pxWindowNative.mm
            mac/pxEventLoopNative.mm mac/pxTimerNative.mm mac/pxTimerNative.mm
            mac/pxClipboardNative.mm mac/window.mm
            unix/rtMutexNative.cpp unix/rtThreadPoolNative.cpp)
    set(PLATFORM_LINKER_FLAGS "-framework Cocoa -framework Foundation")
    include_directories(AFTER ${ZLIBINC} ${CURLINC} ${JPGINC} ${JPGTURBOINC} ${PNGINC})
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -fPIC -g")
    execute_process(COMMAND "hostname" OUTPUT_VARIABLE HOSTNAME)
    string(STRIP ${HOSTNAME} HOSTNAME)
    if (HOSTNAME STREQUAL "raspberrypi")
        message("Building pxcore for raspberry pi")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        add_definitions(-DENABLE_NODE_V_6_9 -DNODE_PLATFORM=linux -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1 -DENABLE_DEBUG_MODE)
        add_definitions(-DPX_PLATFORM_GENERIC_EGL -DRT_PLATFORM_LINUX -DENABLE_RT_NODE -DENABLE_HTTP_CACHE -DRUNINMAIN -DPNG_APNG_SUPPORTED)
        set(PXCORE_FILES gles/pxBufferNative.cpp gles/pxOffscreenNative.cpp gles/pxWindowNative.cpp
                gles/pxEventLoopNative.cpp gles/pxTimerNative.cpp
                gles/pxClipboardNative.cpp gles/pxEGLProviderRPi.cpp gles/LinuxInputEventDispatcher.cpp pxViewWindow.cpp
                unix/rtMutexNative.cpp unix/rtThreadPoolNative.cpp)
        set(PLATFORM_LINKER_FLAGS)
        include_directories(AFTER /opt/vc/include /opt/vc/include/interface/vcos/pthreads
                /opt/vc/include/interface/vmcs_host/linux /opt/vc/include)
        include_directories(AFTER ${ZLIBINC} ${CURLINC} ${JPGINC} ${JPGTURBOINC} ${PNGINC})
    elseif (WINDOWLESS_EGL)
        message("Building pxcore for windowless egl")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)
        add_definitions(-DENABLE_NODE_V_6_9 -DNODE_PLATFORM=linux -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1)
        add_definitions(-DPX_PLATFORM_GENERIC_EGL -DRT_PLATFORM_LINUX -DPX_NO_WINDOW -DENABLE_RT_NODE -DENABLE_HTTP_CACHE -DRUNINMAIN -DRAPIDJSON_HAS_STDSTRING)
        if (DEFINED ENV{PXSCENE_DISABLE_APNG_SUPPORT})
            message("Not including APNG support")
        else ()
            message("Including APNG support")
            add_definitions(-DPNG_APNG_SUPPORTED)
        endif (DEFINED ENV{PXSCENE_DISABLE_APNG_SUPPORT})
        set(PXCORE_STAGING_LIST $ENV{PXCORE_STAGING})
        separate_arguments(PXCORE_STAGING_LIST)
        include_directories(AFTER ${PXCORE_STAGING_LIST})
        set(PXCORE_FILES gles/pxBufferNative.cpp gles/pxOffscreenNative.cpp
                gles/pxEventLoopNative.cpp gles/pxTimerNative.cpp gles/pxClipboardNative.cpp
                unix/rtMutexNative.cpp unix/rtThreadPoolNative.cpp)
        set(PLATFORM_LINKER_FLAGS)
    else ()
        message("Building pxcore for glut")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/glut)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/glut)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/glut)
        add_definitions(-DENABLE_NODE_V_6_9 -DNODE_PLATFORM=linux -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1 -DENABLE_DEBUG_MODE)
        add_definitions(-DPX_PLATFORM_GLUT -DRT_PLATFORM_LINUX -DENABLE_RT_NODE -DENABLE_HTTP_CACHE -DRUNINMAIN -DENABLE_LIBJPEG_TURBO -DPNG_APNG_SUPPORTED)
        set(PXCORE_FILES glut/pxBufferNative.cpp glut/pxOffscreenNative.cpp glut/pxWindowNative.cpp
                glut/pxEventLoopNative.cpp glut/pxTimerNative.cpp glut/pxClipboardNative.cpp pxViewWindow.cpp
                unix/rtMutexNative.cpp unix/rtThreadPoolNative.cpp)
        set(PLATFORM_LINKER_FLAGS)
        include_directories(AFTER ${ZLIBINC} ${CURLINC} ${JPGINC} ${JPGTURBOINC} ${PNGINC} ${GLUTINC})
    endif (HOSTNAME STREQUAL "raspberrypi")
elseif(WIN32)
    message("Building pxcore for Windows")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/win)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/win)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../build/win)
    add_definitions(-DENABLE_NODE_V_6_9 -DNODE_PLATFORM=win32 -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1 -DENABLE_DEBUG_MODE)
    add_definitions(-DENABLE_RT_NODE -DRUNINMAIN -DPNG_APNG_SUPPORTED)
    add_definitions(-DPX_PLATFORM_WIN -DRT_PLATFORM_WINDOWS -DWIN32 -DWIN32_LEAN_AND_MEAN -DGLEW_STATIC -D_TIMESPEC_DEFINED -D_CONSOLE
            -DCURL_STATICLIB -DRUNINMAIN -DENABLE_RT_NODE -DDISABLE_WAYLAND -DNODE_WANT_INTERNALS=1
            -DENABLE_NODE_V_6_9 -DENABLE_V8_HEAP_PARAMS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DHAVE_INSPECTOR=1
            -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1 -DENABLE_DEBUG_MODE -DPNG_APNG_SUPPORTED -DENABLE_MAX_TEXTURE_SIZE
            -DHAVE_OPENSSL -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_HAS_EXCEPTIONS=0 -DBUILDING_V8_SHARED=1
            -DBUILDING_UV_SHARED=1 -DNODE_ARCH="ia32" -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1
            -DHAVE_OPENSSL=1 -DHAVE_ETW=1 -DHAVE_PERFCTR=1 -DFD_SETSIZE=1024 -DNODE_PLATFORM="win32" -D_UNICODE=1 -DUCONFIG_NO_TRANSLITERATION=1
            -DUCONFIG_NO_SERVICE=1 -DUCONFIG_NO_REGULAR_EXPRESSIONS=1 -DU_ENABLE_DYLOAD=0 -DU_STATIC_IMPLEMENTATION=1
            -DU_HAVE_STD_STRING=0 -DUCONFIG_NO_BREAK_ITERATION=0 -DUCONFIG_NO_LEGACY_CONVERSION=1 -DUCONFIG_NO_CONVERSION=1
            -DHTTP_PARSER_STRICT=0 -DUSE_RENDER_STATS -D_HAS_EXCEPTIONS=0)
    #add_definitions(-DRUNINMAIN-DPNG_APNG_SUPPORTED)
    add_definitions(-DWIN32 -D_LIB -DNDEBUG -DPX_PLATFORM_WIN -DRT_PLATFORM_WINDOWS)
    include_directories(AFTER ${ZLIBINC} ${CURLINC} ${JPGINC} ${PNGINC} ${PTHREADINC})
    set(PXCORE_FILES win/pxBufferNative.cpp win/pxOffscreenNative.cpp win/pxWindowNative.cpp
            win/pxEventLoopNative.cpp win/pxTimerNative.cpp win/pxClipboardNative.cpp pxViewWindow.cpp
            win/rtMutexNative.cpp win/rtThreadPoolNative.cpp)
    set_source_files_properties(utf8.c PROPERTIES LANGUAGE CXX)
    set(BUILD_RTCORE_SHARED_LIBRARY 0)
else ()
    message(FATAL_ERROR "Cannot build pxcore.  Unknown platform")
endif (APPLE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive -std=c++11")

if (PXCORE_COMPILE_WARNINGS_AS_ERRORS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif (PXCORE_COMPILE_WARNINGS_AS_ERRORS)

include_directories(AFTER ${NODEPATH}/src ${NODEPATH}/deps/uv/include ${NODEPATH}/deps/v8 ${NODEPATH}/deps/debugger-agent/include ${NODEPATH}/deps/v8/include ${NODEPATH}/deps/cares/include)
include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})

set(PXCORE_FILES ${PXCORE_FILES} pxOffscreen.cpp pxWindowUtil.cpp
        rtFile.cpp rtLibrary.cpp rtNode.cpp
        rtPathUtils.cpp rtTest.cpp rtThreadPool.cpp
        rtThreadQueue.cpp rtThreadTask.cpp rtUrlUtils.cpp
        rtZip.cpp pxInterpolators.cpp pxMatrix4T.cpp pxUtil.cpp
        rtFileDownloader.cpp rtFileCache.cpp rtHttpCache.cpp unzip.c ioapi.c
        jsbindings/jsCallback.cpp jsbindings/rtFunctionWrapper.cpp jsbindings/rtObjectWrapper.cpp
        jsbindings/rtWrapperUtils.cpp)

set(RTCORE_FILES utf8.c rtString.cpp rtLog.cpp rtValue.cpp rtObject.cpp rtError.cpp ioapi_mem.c)

add_library(pxCore ${PXCORE_FILES})
add_library(rtCore_s ${RTCORE_FILES})
if (BUILD_RTCORE_SHARED_LIBRARY GREATER 0)
  add_library(rtCore SHARED ${RTCORE_FILES})
endif(BUILD_RTCORE_SHARED_LIBRARY GREATER 0)
