cmake_minimum_required(VERSION 2.8)
project(pxscene)
find_package(PkgConfig)

set(EXE "Spark")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(PXCOREDIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(EXTDIR "${CMAKE_CURRENT_SOURCE_DIR}/../external")
include(${PXCOREDIR}/cmake/CommOpts.cmake)
include(${PXCOREDIR}/cmake/CommDeps.cmake)
include(${PXCOREDIR}/cmake/NodeDeps.cmake)
include(${PXCOREDIR}/cmake/DukeDeps.cmake)
include(${PXCOREDIR}/cmake/SManDeps.cmake)
if (SUPPORT_V8)
  include(${PXCOREDIR}/cmake/V8Deps.cmake)
endif(SUPPORT_V8)

set(WESTEROSINC ${EXTDIR}/westeros/external/install/include)
set(WESTEROSSTUBINC ${EXTDIR}/westeros-stub)

set(BREAKPADINC ${EXTDIR}/breakpad/src)
set(RTREMOTEINC ${PXCOREDIR}/remote)

set(WINSPARKLEINC ${EXTDIR}/WinSparkle/include)

set(PX_LIBRARY_SUPPORT 1)
set(PXWAYLAND_LIBRARY_SUPPORT 0)
set(PX_LIBRARY_LINK_PXCORE 1)

option(BUILD_WITH_GL "BUILD_WITH_GL" ON)
option(BUILD_WITH_PXPATH "BUILD_WITH_PXPATH" OFF)
option(BUILD_WITH_WAYLAND "BUILD_WITH_WAYLAND" OFF)
option(BUILD_WITH_WESTEROS "BUILD_WITH_WESTEROS" OFF)
option(BUILD_WITH_CXX_11 "BUILD_WITH_CXX_11" ON)
option(BUILD_WITH_TEXTURE_USAGE_MONITORING "BUILD_WITH_TEXTURE_USAGE_MONITORING" OFF)
option(BUILD_WITH_WINDOWLESS_EGL "BUILD_WITH_WINDOWLESS_EGL" OFF)
option(BUILD_PXSCENE_WAYLAND_EGL "BUILD_PXSCENE_WAYLAND_EGL" OFF)
option(BUILD_PXSCENE_ESSOS "BUILD_PXSCENE_ESSOS" OFF)
option(BUILD_WITH_WINDOWLESS_DFB "BUILD_WITH_WINDOWLESS_DFB" OFF)
option(BUILD_WITH_SERVICE_MANAGER "BUILD_WITH_SERVICE_MANAGER" OFF)
option(BUILD_WITH_SERVICE_MANAGER_LINKED "BUILD_WITH_SERVICE_MANAGER_LINKED" OFF)
option(BUILD_PXSCENE_APP "BUILD_PXSCENE_APP" ON)
option(BUILD_PXSCENE_SHARED_LIB "BUILD_PXSCENE_SHARED_LIB" OFF)
option(BUILD_PXSCENE_STATIC_LIB "BUILD_PXSCENE_STATIC_LIB" OFF)
option(BUILD_PXWAYLAND_SHARED_LIB "BUILD_PXWAYLAND_SHARED_LIB" OFF)
option(BUILD_PXWAYLAND_STATIC_LIB "BUILD_PXWAYLAND_STATIC_LIB" ON)
option(BUILD_OPTIMUS_STATIC_LIB "BUILD_OPTIMUS_STATIC_LIB" OFF)
option(BUILD_PXSCENE_WITH_NEXUS_SUPPORT "BUILD_PXSCENE_WITH_NEXUS_SUPPORT" OFF)
option(BUILD_DEBUG_METRICS "BUILD_DEBUG_METRICS" OFF)
option(BUILD_WITH_STATIC_NODE "BUILD_WITH_STATIC_NODE" OFF)
option(BUILD_PXSCENE_APP_WITH_PXSCENE_LIB "BUILD_PXSCENE_APP_WITH_PXSCENE_LIB" OFF)
option(PXSCENE_ACCESS_CONTROL_CHECK "PXSCENE_ACCESS_CONTROL_CHECK" ON)
option(PXSCENE_CORS_FOR_RESOURCES "PXSCENE_CORS_FOR_RESOURCES" ON)
option(PXSCENE_PERMISSIONS_CHECK "PXSCENE_PERMISSIONS_CHECK" ON)
option(PXSCENE_DIRTY_RECTANGLES "PXSCENE_DIRTY_RECTANGLES" OFF)
option(SPARK_ENABLE_LRU_TEXTURE_EJECTION "SPARK_ENABLE_LRU_TEXTURE_EJECTION" ON)
option(SPARK_BACKGROUND_TEXTURE_CREATION "SPARK_BACKGROUND_TEXTURE_CREATION" OFF)
option(SPARK_ENABLE_ALPHA_FBO_SUPPORT "SPARK_ENABLE_ALPHA_FBO_SUPPORT" ON)

if(WIN32)
    option(PXSCENE_COMPILE_WARNINGS_AS_ERRORS "PXSCENE_COMPILE_WARNINGS_AS_ERRORS" OFF)
elseif (APPLE)
    option(PXSCENE_COMPILE_WARNINGS_AS_ERRORS "PXSCENE_COMPILE_WARNINGS_AS_ERRORS" ON)
else()
    option(PXSCENE_COMPILE_WARNINGS_AS_ERRORS "PXSCENE_COMPILE_WARNINGS_AS_ERRORS" ON)
endif(WIN32)

option(PXSCENE_FONT_ATLAS "PXSCENE_FONT_ATLAS" ON)

if (PXSCENE_FONT_ATLAS)
    message("Building with Font Atlas Support")
    add_definitions(-DPXSCENE_FONT_ATLAS)
endif (PXSCENE_FONT_ATLAS)

set(PXSCENE_DEFINITIONS )
set(PXSCENE_ADDITIONAL_RESOURCES )
set(PXSCENE_INSTALLER 0)

if (APPLE)
    message("building pxscene for mac")
    set(PX_PLATFORM PX_PLATFORM_MAC)
    set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DRT_PLATFORM_LINUX)
    set(PXCORE_LIB_LOCATION ${PXCOREDIR}/build/mac)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fpermissive -g -Wall -Wno-attributes -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive -g -Wall -Wno-attributes -Wall -Wextra")
    set(PXSCENE_LINK_DIRECTORIES ${PXCORE_LIB_LOCATION})
    set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DENABLE_NODE_V_6_9 -D_DARWIN_USE_64_BIT_INODE=1 -DNODE_ARCH="x64" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS= -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1 -DHAVE_OPENSSL=1 -DHAVE_DTRACE=1 -D__POSIX__ -DNODE_PLATFORM=darwin -DDUCONFIG_NO_TRANSLITERATION=1 -DUCONFIG_NO_SERVICE=1 -DUCONFIG_NO_REGULAR_EXPRESSIONS=1 -DU_ENABLE_DYLOAD=0 -DU_STATIC_IMPLEMENTATION=1 -DU_HAVE_STD_STRING=0 -DUCONFIG_NO_BREAK_ITERATION=0 -DUCONFIG_NO_LEGACY_CONVERSION=1 -DUCONFIG_NO_CONVERSION=1 -DHTTP_PARSER_STRICT=0 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64)
    set(PLATFORM_SOURCES mac/pxContextUtils.mm)
    set(PXSCENE_LINKER_OPTIONS "-framework OpenGL -framework Cocoa -framework Foundation")
    if (ENABLE_THREAD_SANITIZER)
      set(PXSCENE_LINKER_OPTIONS "${PXSCENE_LINKER_OPTIONS} -fsanitize=thread")
    endif (ENABLE_THREAD_SANITIZER)
    if (ENABLE_ADDRESS_SANITIZER)
        set(PXSCENE_LINKER_OPTIONS "${PXSCENE_LINKER_OPTIONS} -fsanitize=address")
    endif (ENABLE_ADDRESS_SANITIZER)
    set(PXSCENE_LINK_LIBRARIES)
    set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${NODE_LIBRARY_DIRS} ${V8_LIBRARY_DIRS} ${DUKE_LIBRARY_DIRS} ${COMM_DEPS_LIBRARY_DIRS})
    set(PLATFORM_LIBRARIES pxCore rtCore_s pthread ${NODE_LIBRARIES} ${V8_LIBRARIES} ${DUKE_LIBRARIES} ${COMM_DEPS_LIBRARIES})
    include_directories(AFTER ${NODE_INCLUDE_DIRS} ${DUKE_INCLUDE_DIRS} ${COMM_DEPS_INCLUDE_DIRS})
    if (DEFINED ENV{CODE_COVERAGE})
        message("enabling code coverage support")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline")
        set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DENABLE_CODE_COVERAGE)
        execute_process(COMMAND clang --version OUTPUT_VARIABLE LLVM_VERSION1 ERROR_VARIABLE ERROR_VARIABLE1 COMMAND grep "LLVM version" OUTPUT_VARIABLE LLVM_VERSION2 ERROR_VARIABLE LLVM_ERROR2 COMMAND awk "{print $4}" OUTPUT_FILE llvmoutput OUTPUT_VARIABLE LLVM_VERSION3 ERROR_VARIABLE LLVM_ERROR3)
        execute_process(COMMAND awk "{ printf \"%s\", $0 }" llvmoutput OUTPUT_VARIABLE LLVM_VERSION)
        execute_process(COMMAND rm llvmoutput)
        set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/${LLVM_VERSION}/lib/darwin)
        set(PXSCENE_LINK_LIBRARIES ${PXSCENE_LINK_LIBRARIES} clang_rt.profile_osx)
    endif ()
    #set(PX_LIBRARY_SUPPORT 0)
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -fpermissive -g -Wall -Wno-attributes -Wall -Wextra")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive -g -Wall -Wno-attributes -Wall -Wextra")
    execute_process(COMMAND "hostname" OUTPUT_VARIABLE HOSTNAME)
    string(STRIP ${HOSTNAME} HOSTNAME)
    set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DRT_PLATFORM_LINUX)
    add_definitions(${COMM_DEPS_DEFINITIONS})

    include_directories(AFTER
               ${V8_INCLUDE_DIRS}
               ${NODE_INCLUDE_DIRS}
               ${DUKE_INCLUDE_DIRS}
          ${COMM_DEPS_INCLUDE_DIRS}
    )

    set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES}
               ${NODE_LIBRARY_DIRS}
               ${V8_LIBRARY_DIRS}
               ${DUKE_LIBRARY_DIRS}
          ${COMM_DEPS_LIBRARY_DIRS}
       )
    set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES}
                  ${NODE_LIBRARIES}
                  ${V8_LIBRARIES}
                  ${DUKE_LIBRARIES}
             ${COMM_DEPS_LIBRARIES}
             pthread rt dl m)

    set(PXSCENE_APP_LIBRARIES ${PXSCENE_APP_LIBRARIES} pxCore rtCore_s)

    if (HOSTNAME STREQUAL "raspberrypi")
        message("building pxscene for raspberrypi")
        set(PX_PLATFORM PX_PLATFORM_GENERIC_EGL)
        set(PXCORE_LIB_LOCATION ${PXCOREDIR}/build/egl)
        set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${PXCORE_LIB_LOCATION})
        set(PLATFORM_SOURCES egl/pxContextUtils.cpp)
        set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DPXSCENE_DISABLE_PXCONTEXT_EXT)
        include_directories(AFTER /opt/vc/include /opt/vc/include/interface/vcos/pthreads
                /opt/vc/include/interface/vmcs_host/linux /opt/vc/include)
        set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} /opt/vc/lib/)
        set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} bcm_host brcmEGL brcmGLESv2)
    elseif (BUILD_WITH_WINDOWLESS_EGL OR BUILD_PXSCENE_WAYLAND_EGL OR BUILD_PXSCENE_ESSOS)
        if (BUILD_WITH_WINDOWLESS_EGL)
            message("Building pxscene for windowless EGL")
            set(PX_PLATFORM PX_PLATFORM_GENERIC_EGL)
            set(PXSCENE_ADDITIONAL_RESOURCES ${PXSCENE_ADDITIONAL_RESOURCES} ${PXCOREDIR}/src/gles/pxWindowNative.cpp
                  ${PXCOREDIR}/src/gles/LinuxInputEventDispatcher.cpp)
        elseif (BUILD_PXSCENE_ESSOS)
            message("Building pxscene with essos windows")
            set(PX_PLATFORM PX_PLATFORM_WAYLAND_EGL)
            set(PXSCENE_APP_LIBRARIES ${PXSCENE_APP_LIBRARIES} essos)
        elseif (BUILD_PXSCENE_WAYLAND_EGL)
            message("Building pxscene with wayland windows")
            set(PX_PLATFORM PX_PLATFORM_WAYLAND_EGL)
            set(PXSCENE_APP_LIBRARIES ${PXSCENE_APP_LIBRARIES} ${WAYLAND_EGL_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES})
            set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${WAYLAND_EGL_LIBRARY_DIRS} ${WAYLAND_CLIENT_LIBRARY_DIRS})
        endif(BUILD_WITH_WINDOWLESS_EGL)

        set(PXCORE_LIB_LOCATION ${PXCOREDIR}/build/egl)
        set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${PXCORE_LIB_LOCATION} ${PXCOREDIR}/remote ${CMAKE_CURRENT_SOURCE_DIR})
        set(PLATFORM_SOURCES egl/pxContextUtils.cpp)

        if (BUILD_PXSCENE_WITH_NEXUS_SUPPORT)
            set(PXSCENE_APP_LIBRARIES ${PXSCENE_APP_LIBRARIES} nxpl)
            set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} nexus nxclient)
            set(PXSCENE_ADDITIONAL_RESOURCES ${PXSCENE_ADDITIONAL_RESOURCES} ${PXCOREDIR}/src/gles/pxEGLProviderNexus.cpp)
        endif(BUILD_PXSCENE_WITH_NEXUS_SUPPORT)

        set(PXSCENE_ADDITIONAL_APP_INCLUDES_LIST $ENV{PXSCENE_ADDITIONAL_APP_INCLUDES})
        separate_arguments(PXSCENE_ADDITIONAL_APP_INCLUDES_LIST)
        include_directories(AFTER ${PXSCENE_ADDITIONAL_APP_INCLUDES_LIST})
        set(PXSCENE_ADDITIONAL_APP_LIBRARIES_LIST $ENV{PXSCENE_ADDITIONAL_APP_LIBRARIES})
        separate_arguments(PXSCENE_ADDITIONAL_APP_LIBRARIES_LIST)
        set(PXSCENE_APP_LIBRARIES ${PXSCENE_APP_LIBRARIES} ${PXSCENE_ADDITIONAL_APP_LIBRARIES_LIST})

        if (BUILD_WITH_SERVICE_MANAGER_LINKED)
          include_directories(AFTER                                ${SMAN_DEPS_INCLUDE_DIRS})
          set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${SMAN_DEPS_LIBRARY_DIRS})
          set(PXSCENE_APP_LIBRARIES ${PXSCENE_APP_LIBRARIES} rtRemote ${SMAN_DEPS_LIBRARIES})
        endif (BUILD_WITH_SERVICE_MANAGER_LINKED)

        set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DPX_NO_WINDOW -DENABLE_V8_HEAP_PARAMS -DENABLE_MAX_TEXTURE_SIZE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE)
        set(PXSCENE_PLATFORM_DEFINE_LIST)
        if (DEFINED ENV{PXSCENE_PLATFORM_DEFINES})
            set(PXSCENE_PLATFORM_DEFINE_LIST $ENV{PXSCENE_PLATFORM_DEFINES})
            separate_arguments(PXSCENE_PLATFORM_DEFINE_LIST)
            set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} ${PXSCENE_PLATFORM_DEFINE_LIST})
            message("Extra platform defines: " ${PXSCENE_PLATFORM_DEFINE_LIST})
        endif (DEFINED ENV{PXSCENE_PLATFORM_DEFINES})

        set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DPXSCENE_ENABLE_ALL_APPS_WAYLAND_CONFIG)
        set(PXWAYLAND_LIBRARY_SUPPORT 1)
        include_directories(AFTER ${WAYLAND_EGL_INCLUDE_DIRS})
        set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES})
    elseif (BUILD_WITH_WINDOWLESS_DFB)
        message("building pxscene for windowless DFB")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GNU_SOURCE=1 -D__STDC_LIMIT_MACROS -O3")
        set(PX_PLATFORM PX_PLATFORM_GENERIC_DFB)
        set(PXCORE_LIB_LOCATION ${PXCOREDIR}/build/dfb)
        set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${PXCORE_LIB_LOCATION})
        set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} directfb direct fusion)
        set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DPX_NO_WINDOW -DENABLE_DFB -DENABLE_DFB_GENERIC -DPX_PLATFORM_GENERIC_DFB -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE)
        set(PXCORE_INCLUDES $ENV{PXCORE_INCLUDES})
        separate_arguments(PXCORE_INCLUDES)
        include_directories(BEFORE ${PXCORE_INCLUDES})
    else ()
        message("building pxscene for glut")
        set(PX_PLATFORM PX_PLATFORM_GLUT)
        set(PXCORE_LIB_LOCATION ${PXCOREDIR}/build/glut)
        set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${PXCORE_LIB_LOCATION})
        set(PLATFORM_SOURCES glut/pxContextUtils.cpp)
        set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DHAS_LINUX_BREAKPAD)
        include_directories(AFTER ${GLUTINC})
        set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} glut breakpad_client)
        set(PX_LIBRARY_LINK_PXCORE 0)
    endif (HOSTNAME STREQUAL "raspberrypi")
    if (NOT USE_NODE_0_12_7)
    set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1)
    endif (NOT USE_NODE_0_12_7)
    set(PXSCENE_LINKER_OPTIONS "")
    #set(PXSCENE_LINK_LIBRARIES pthread)
    if (DEFINED ENV{CODE_COVERAGE})
        message("enabling code coverage support")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -fno-inline")
        set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DENABLE_CODE_COVERAGE)
        set(PXSCENE_LINK_LIBRARIES ${PXSCENE_LINK_LIBRARIES} gcov)
    endif ()
elseif (WIN32)
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)
    add_definitions(-DWIN32 -D_LIB -DPX_PLATFORM_WIN -DRT_PLATFORM_WINDOWS)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT ")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /EHs-c-")
    set(PXCORE_LIB_LOCATION ${PXCOREDIR}/build/win/Release)
    set(PXSCENE_LINK_DIRECTORIES ${PXCORE_LIB_LOCATION})
    set(PLATFORM_SOURCES glut/pxContextUtils.cpp)
    if (SUPPORT_NODE)
        set(PLATFORM_SOURCES ${PLATFORM_SOURCES}
    	    ${NODEDIR}/src/async-wrap.cc ${NODEDIR}/src/backtrace_win32.cc
    	    ${NODEDIR}/src/cares_wrap.cc ${NODEDIR}/src/connection_wrap.cc ${NODEDIR}/src/connect_wrap.cc
    	    ${NODEDIR}/src/debug-agent.cc ${NODEDIR}/src/env.cc ${NODEDIR}/src/fs_event_wrap.cc ${NODEDIR}/src/handle_wrap.cc
    	    ${NODEDIR}/src/inspector_agent.cc ${NODEDIR}/src/inspector_socket.cc ${NODEDIR}/src/js_stream.cc ${NODEDIR}/src/node.cc
    	    ${NODEDIR}/src/node_buffer.cc ${NODEDIR}/src/node_config.cc ${NODEDIR}/src/node_constants.cc
    	    ${NODEDIR}/src/node_contextify.cc ${NODEDIR}/src/node_counters.cc ${NODEDIR}/src/node_crypto.cc
    	    ${NODEDIR}/src/node_crypto_bio.cc ${NODEDIR}/src/node_crypto_clienthello.cc ${NODEDIR}/src/node_dtrace.cc
    	    ${NODEDIR}/src/node_file.cc ${NODEDIR}/src/node_http_parser.cc ${NODEDIR}/src/node_i18n.cc
    	    ${NODEDIR}/src/node_javascript.cc ${NODEDIR}/src/node_os.cc ${NODEDIR}/src/node_revert.cc
    	    ${NODEDIR}/src/node_stat_watcher.cc ${NODEDIR}/src/node_util.cc ${NODEDIR}/src/node_v8.cc ${NODEDIR}/src/node_watchdog.cc
    	    ${NODEDIR}/src/node_win32_etw_provider.cc ${NODEDIR}/src/node_win32_perfctr_provider.cc ${NODEDIR}/src/node_zlib.cc
    	    ${NODEDIR}/src/pipe_wrap.cc ${NODEDIR}/src/process_wrap.cc ${NODEDIR}/src/signal_wrap.cc ${NODEDIR}/src/spawn_sync.cc
    	    ${NODEDIR}/src/stream_base.cc ${NODEDIR}/src/stream_wrap.cc ${NODEDIR}/src/string_bytes.cc ${NODEDIR}/src/string_search.cc
    	    ${NODEDIR}/src/tcp_wrap.cc ${NODEDIR}/src/timer_wrap.cc ${NODEDIR}/src/tls_wrap.cc ${NODEDIR}/src/tty_wrap.cc
    	    ${NODEDIR}/src/udp_wrap.cc ${NODEDIR}/src/util.cc ${NODEDIR}/src/uv.cc)
    endif (SUPPORT_NODE)
    set(PLATFORM_SOURCES ${PLATFORM_SOURCES} ${PXCOREDIR}/src/utf8.c)
    set_source_files_properties(utf8.c PROPERTIES LANGUAGE CXX)
    set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DHAS_WINDOWS_BREAKPAD)
    add_definitions(-DPX_PLATFORM_WIN -DRT_PLATFORM_WINDOWS -DWIN32 -DWIN32_LEAN_AND_MEAN -DGLEW_STATIC -D_TIMESPEC_DEFINED -D_CONSOLE
            -DCURL_STATICLIB -DRUNINMAIN -DENABLE_RT_NODE -DDISABLE_WAYLAND -DNODE_WANT_INTERNALS=1
            -DENABLE_NODE_V_6_9 -DENABLE_V8_HEAP_PARAMS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DHAVE_INSPECTOR=1
            -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1 -DENABLE_MAX_TEXTURE_SIZE
            -DHAVE_OPENSSL -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_HAS_EXCEPTIONS=0 -DBUILDING_V8_SHARED=1
            -DBUILDING_UV_SHARED=1 -DNODE_ARCH="ia32" -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1
            -DHAVE_OPENSSL=1 -DHAVE_ETW=1 -DHAVE_PERFCTR=1 -DFD_SETSIZE=1024 -DNODE_PLATFORM="win32" -D_UNICODE=1 -DUCONFIG_NO_TRANSLITERATION=1
            -DUCONFIG_NO_SERVICE=1 -DUCONFIG_NO_REGULAR_EXPRESSIONS=1 -DU_ENABLE_DYLOAD=0 -DU_STATIC_IMPLEMENTATION=1
            -DU_HAVE_STD_STRING=0 -DUCONFIG_NO_BREAK_ITERATION=0 -DUCONFIG_NO_LEGACY_CONVERSION=1 -DUCONFIG_NO_CONVERSION=1
            -DHTTP_PARSER_STRICT=0 -DUSE_RENDER_STATS -D_HAS_EXCEPTIONS=0)
    add_definitions(${COMM_DEPS_DEFINITIONS})
    include_directories(AFTER ${COMM_DEPS_INCLUDE_DIRS} ${NODE_INCLUDE_DIRS} ${V8_INCLUDE_DIRS} ${DUKE_INCLUDE_DIRS})
    include_directories(AFTER "${EXTDIR}/pthread-2.9" ${WINSPARKLEINC} ${EXTDIR}/breakpad-chrome_55/src/)

    set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${COMM_DEPS_LIBRARY_DIRS} ${NODE_LIBRARY_DIRS}
        ${V8_LIBRARY_DIRS} ${DUKE_LIBRARY_DIRS})

    set(PLATFORM_LIBRARIES pxCore.lib)
    set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} ${COMM_DEPS_LIBRARIES} ${NODE_LIBRARIES} ${V8_LIBRARIES} ${DUKE_LIBRARIES})
    set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES}
        pthreadVC2.lib opengl32.lib Winmm.lib Shlwapi.lib
        Ws2_32.lib Wldap32.lib psapi.lib
        iphlpapi.lib userenv.lib rtCore_s.lib common.lib
        crash_generation_client.lib exception_handler.lib)
  
    if(SUPPORT_DUKTAPE)
      set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} msvcrt.lib)
    endif(SUPPORT_DUKTAPE)


    set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${EXTDIR}/breakpad-chrome_55/src/client/windows/Release/lib)

    set(PXSCENE_ADDITIONAL_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/win/pxscene.rc)
    set(PX_LIBRARY_LINK_PXCORE 0)
    set(PX_LIBRARY_SUPPORT 0)
    set(PXSCENE_LINKER_OPTIONS " /DEBUG ")
    set(PXSCENE_INSTALLER 1)
else ()
    message(FATAL_ERROR "Cannot build pxscene. Unknown platform")
endif (APPLE)

if (DEFINED ENV{PX_BUILD_VERSION})
    set(PXSCENE_VERSION $ENV{PX_BUILD_VERSION})
endif(DEFINED ENV{PX_BUILD_VERSION})

if ("${PXSCENE_VERSION}" STREQUAL "")
    add_definitions(-DPX_SCENE_VERSION=dev)
    set(PXSCENE_VERSION dev)
else ()
    message("pxscene version is: ${PXSCENE_VERSION}")
    add_definitions(-DPX_SCENE_VERSION=${PXSCENE_VERSION})
endif("${PXSCENE_VERSION}" STREQUAL "")

set(TARGETINCLUDE "")
set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${EXTDIR}/breakpad/src/client/linux)

set(PXSCENE_LINK_LIBRARIES ${PXSCENE_LINK_LIBRARIES} ${PLATFORM_LIBRARIES})

set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -D${PX_PLATFORM} -DENABLE_RT_NODE -DRUNINMAIN -DENABLE_HTTP_CACHE)

#todo add code coverage check
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
#set(PXSCENE_LINKER_OPTIONS "${PXSCENE_LINKER_OPTIONS} -lgcov")

include_directories(AFTER ${NODE_INCLUDE_DIRS} ${V8_INCLUDE_DIRS} ${DUKE_INCLUDE_DIRS})
include_directories(AFTER ${WESTEROSINC} ${WESTEROSSTUBINC} ${TURBO_JPEG_INCLUDE_DIRS} ${BREAKPADINC} ${RTREMOTEINC} ${PXCOREDIR}/src)
include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(AFTER ${CMAKE_CURRENT_SOURCE_DIR}/rasterizer)

set(PXSCENE_COMMON_FILES pxResource.cpp pxConstants.cpp pxRectangle.cpp pxFont.cpp pxText.cpp
        pxTextBox.cpp pxImage.cpp pxImage9.cpp pxImageA.cpp pxImage9Border.cpp pxArchive.cpp pxAnimate.cpp)

if (BUILD_WITH_PXPATH)
    message("Building with pxPath support")
    set(PXSCENE_COMMON_FILES  rasterizer/pxPath.cpp)
endif (BUILD_WITH_PXPATH)

set(PXSCENE_COMMON_FILES ${PXSCENE_COMMON_FILES} pxScene2d.cpp)

set(PXWAYLAND_LIB_FILES pxContextGL.cpp egl/pxContextUtils.cpp)

if (BUILD_WITH_GL)
    message("Building with GL support")
    set(PXSCENE_COMMON_FILES ${PXSCENE_COMMON_FILES} pxContextGL.cpp)
else ()
    message("Building with DirectFB support")
    set(PXSCENE_COMMON_FILES ${PXSCENE_COMMON_FILES} pxContextDFB.cpp)
endif (BUILD_WITH_GL)

if (BUILD_WITH_WAYLAND)
    message("Building with wayland support")
    set(PXSCENE_COMMON_FILES ${PXSCENE_COMMON_FILES} pxWayland.cpp pxWaylandContainer.cpp)
    set(PXWAYLAND_LIB_FILES ${PXWAYLAND_LIB_FILES} pxWayland.cpp)
    if (BUILD_RTREMOTE_LIBS)
        set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DENABLE_PX_WAYLAND_RPC)
        set(PXSCENE_APP_LIBRARIES ${PXSCENE_APP_LIBRARIES} rtremote_shared)
        set(PXSCENE_LINK_DIRECTORIES ${PXSCENE_LINK_DIRECTORIES} ${RTREMOTEINC})
    endif (BUILD_RTREMOTE_LIBS)
else ()
    message("Not building with wayland support")
    set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DDISABLE_WAYLAND)
endif (BUILD_WITH_WAYLAND)

if (BUILD_WITH_WESTEROS)
    message("Building with Westeros support")
    set(PXSCENE_LINK_LIBRARIES ${PXSCENE_LINK_LIBRARIES} westeros_compositor)
else ()
    message("Building with Westeros stubs")
    set(PXSCENE_COMMON_FILES ${PXSCENE_COMMON_FILES} ../external/westeros-stub/westeros-stub.cpp)
endif (BUILD_WITH_WESTEROS)

if (BUILD_WITH_SERVICE_MANAGER)
    message("Building with Service Manager support")
    set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DPX_SERVICE_MANAGER)
    set(PXSCENE_COMMON_FILES ${PXSCENE_COMMON_FILES} pxServiceManager.cpp)
endif (BUILD_WITH_SERVICE_MANAGER)

if (BUILD_WITH_SERVICE_MANAGER_LINKED)
    message("Building with Service Manager API library support")
    set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DPX_SERVICE_MANAGER_LINKED)
endif (BUILD_WITH_SERVICE_MANAGER_LINKED)

if (BUILD_OPTIMUS_STATIC_LIB)
    message("Building with Optimus support")
    set(PXSCENE_APP_LIBRARIES ${PXSCENE_APP_LIBRARIES} optimus rtRemote)
    add_definitions(-DENABLE_OPTIMUS_SUPPORT)
endif (BUILD_OPTIMUS_STATIC_LIB)

if (BUILD_DEBUG_METRICS)
    message("Building with debug metrics")
    add_definitions(-DENABLE_DEBUG_METRICS)
endif (BUILD_DEBUG_METRICS)

if (SPARK_ENABLE_LRU_TEXTURE_EJECTION)
    message("Building with LRU texture ejection")
    add_definitions(-DENABLE_LRU_TEXTURE_EJECTION)
endif (SPARK_ENABLE_LRU_TEXTURE_EJECTION)

if (SPARK_BACKGROUND_TEXTURE_CREATION)
    message("Building with background texture creation")
    add_definitions(-DENABLE_BACKGROUND_TEXTURE_CREATION)
endif (SPARK_BACKGROUND_TEXTURE_CREATION)

if (SPARK_ENABLE_ALPHA_FBO_SUPPORT)
    message("Building with alpha fbo support")
    add_definitions(-DSPARK_ALPHA_FBO_SUPPORT)
endif (SPARK_ENABLE_ALPHA_FBO_SUPPORT)

set(PXSCENE_COMMON_FILES ${PXSCENE_COMMON_FILES} ${PLATFORM_SOURCES})

set(PXSCENE_APP_FILES ${PXSCENE_COMMON_FILES} Spark.cpp)
set(PXSCENE_LIB_FILES ${PXSCENE_COMMON_FILES})

if (PXSCENE_COMPILE_WARNINGS_AS_ERRORS)
  if (!APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
  else ()
    message("Cleanup OSX build!!")
  endif ()
endif (PXSCENE_COMPILE_WARNINGS_AS_ERRORS)

if (BUILD_WITH_CXX_11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif (BUILD_WITH_CXX_11)

execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE SPARK_GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("Building Spark with git revision ${SPARK_GIT_REVISION}")

set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DSPARK_BUILD_GIT_REVISION="${SPARK_GIT_REVISION}")

if (BUILD_WITH_TEXTURE_USAGE_MONITORING)
    set(PXSCENE_DEFINITIONS ${PXSCENE_DEFINITIONS} -DENABLE_PX_SCENE_TEXTURE_USAGE_MONITORING)
endif (BUILD_WITH_TEXTURE_USAGE_MONITORING)

set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${PXSCENE_LINKER_OPTIONS})
link_directories(${PXSCENE_LINK_DIRECTORIES})

if (BUILD_PXSCENE_APP)
    message("Enabling build support for pxscene app")
    add_executable(pxscene_app ${PXSCENE_APP_FILES} ${PXSCENE_ADDITIONAL_RESOURCES})
    set_target_properties(pxscene_app PROPERTIES OUTPUT_NAME ${EXE})
    target_link_libraries(pxscene_app ${PXSCENE_APP_LIBRARIES} ${PXSCENE_LINK_LIBRARIES})
    add_definitions(${PXSCENE_DEFINITIONS})

    if (APPLE)
        add_custom_command(TARGET pxscene_app COMMENT "making app bundle" POST_BUILD COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mkapp.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif (APPLE)

    if (PX_LIBRARY_SUPPORT GREATER 0)
        if (BUILD_PXSCENE_SHARED_LIB OR BUILD_PXSCENE_SHARED_LIB)
            add_dependencies(pxscene_app pxscene)
        endif (BUILD_PXSCENE_SHARED_LIB OR BUILD_PXSCENE_SHARED_LIB)
    endif (PX_LIBRARY_SUPPORT GREATER 0)

    if (BUILD_OPTIMUS_STATIC_LIB)
      add_dependencies(pxscene_app optimus)
    endif (BUILD_OPTIMUS_STATIC_LIB)

    if (PXSCENE_INSTALLER GREATER 0)
      if (WIN32)
        message("enabling installer support")
        install(TARGETS pxscene_app RUNTIME DESTINATION . COMPONENT pxscene)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/browser DESTINATION . COMPONENT pxscene)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/node_modules DESTINATION . COMPONENT pxscene)
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/v8_modules DESTINATION . COMPONENT pxscene)
        if (SUPPORT_DUKTAPE)
            install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/rcvrcore/" DESTINATION rcvrcore COMPONENT pxscene)
            install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/duk_modules/" DESTINATION duk_modules COMPONENT pxscene)
            file(GLOB PXSCENE_INSTALL_FILES_JS "${CMAKE_CURRENT_SOURCE_DIR}/*.js")
            file(GLOB PXSCENE_INSTALL_FILES_JS2 "${CMAKE_CURRENT_SOURCE_DIR}/duktape/*.js")
        else (SUPPORT_DUKTAPE)
            install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/rcvrcore DESTINATION . COMPONENT pxscene)
            file(GLOB PXSCENE_INSTALL_FILES_JS "${CMAKE_CURRENT_SOURCE_DIR}/*.js")
        endif (SUPPORT_DUKTAPE)

        file(GLOB PXSCENE_INSTALL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/FreeSans.ttf"
           "${CMAKE_CURRENT_SOURCE_DIR}/*.json" "${CMAKE_CURRENT_SOURCE_DIR}/*.conf"
           "${CMAKE_CURRENT_SOURCE_DIR}/../external/vc.build/builds/*.dll" 
           "${CMAKE_CURRENT_SOURCE_DIR}/../external/v8/out.gn/ia32.release/v8*.dll"
           "${CMAKE_CURRENT_SOURCE_DIR}/Release/")
        if (SUPPORT_DUKTAPE)
            message("installing additional duktape files")
            install(FILES ${PXSCENE_INSTALL_FILES_JS2} DESTINATION . COMPONENT pxscene)
        endif (SUPPORT_DUKTAPE)
        install(FILES ${PXSCENE_INSTALL_FILES_JS} ${PXSCENE_INSTALL_FILES} DESTINATION . COMPONENT pxscene)
        set(CPACK_PACKAGE_VERSION "${PXSCENE_VERSION}")
        set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Spark.lnk' '$INSTDIR\\\\Spark.exe' ''")
        set(CPACK_NSIS_DELETE_ICONS_EXTRA "Delete '$SMPROGRAMS\\\\$START_MENU\\\\Spark.lnk'")
        set(CPACK_PACKAGE_FILE_NAME "spark-setup")
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "Spark")
        set(CPACK_RESOURCE_FILE_LICENSE "${PXCOREDIR}/LICENSE.txt")
        set(CPACK_PACKAGE_VENDOR "pxscene.org")
        set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/win/Spark_installer.ico")
        set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/win/Spark_installer.ico")
        include(CPack)
      endif (WIN32)
    endif (PXSCENE_INSTALLER GREATER 0)
endif (BUILD_PXSCENE_APP)

if (PXSCENE_ACCESS_CONTROL_CHECK)
    message("Enabling CORS")
    add_definitions(-DENABLE_ACCESS_CONTROL_CHECK)
endif (PXSCENE_ACCESS_CONTROL_CHECK)

if (PXSCENE_CORS_FOR_RESOURCES)
    message("Enabling CORS for resources")
    add_definitions(-DENABLE_CORS_FOR_RESOURCES)
endif (PXSCENE_CORS_FOR_RESOURCES)

if (PXSCENE_DIRTY_RECTANGLES)
    message("Enabling dirty rectangles support")
    add_definitions(-DPX_DIRTY_RECTANGLES)
endif(PXSCENE_DIRTY_RECTANGLES)

if (PXSCENE_PERMISSIONS_CHECK)
    message("Enabling permissions")
    add_definitions(-DENABLE_PERMISSIONS_CHECK)
endif (PXSCENE_PERMISSIONS_CHECK)

if (PX_LIBRARY_SUPPORT GREATER 0)
    message("Enabling build support for libraries")
    if (PX_LIBRARY_LINK_PXCORE GREATER 0)
        if (NOT APPLE)
            set(LIBRARY_LINKER_OPTIONS -Wl,--whole-archive ${PXCORE_LIB_LOCATION}/libpxCore.a -Wl,--no-whole-archive)
        endif (NOT APPLE)
    endif (PX_LIBRARY_LINK_PXCORE GREATER 0)

    if (BUILD_PXSCENE_STATIC_LIB)
        message("Enabling build support for pxscene static libraries")
        add_library(pxscene_static ${PXSCENE_LIB_FILES})
        if (${CMAKE_VERSION} VERSION_GREATER 2.8.11 )
          add_library(Spark ALIAS pxscene_static)
        endif ()
        set_target_properties(pxscene_static PROPERTIES OUTPUT_NAME ${EXE})
        target_link_libraries(pxscene_static ${LIBRARY_LINKER_OPTIONS} ${PXSCENE_LINK_LIBRARIES})
        add_definitions(${PXSCENE_DEFINITIONS})
        if (PX_LIBRARY_LINK_PXCORE GREATER 0)
            add_dependencies(pxscene_static pxCore rtCore)
        endif (PX_LIBRARY_LINK_PXCORE GREATER 0)
    endif (BUILD_PXSCENE_STATIC_LIB)

    if (BUILD_PXSCENE_SHARED_LIB)
        message("Enabling build support for pxscene shared libraries")
        add_library(pxscene_shared SHARED ${PXSCENE_LIB_FILES})
        if (${CMAKE_VERSION} VERSION_GREATER 2.8.11 )
          add_library(Spark ALIAS pxscene_shared)
        endif ()
        if (SUPPORT_DUKTAPE)
          add_dependencies (pxscene_shared dukluv)
        endif (SUPPORT_DUKTAPE)
        set_target_properties(pxscene_shared PROPERTIES OUTPUT_NAME ${EXE})
        target_link_libraries(pxscene_shared ${LIBRARY_LINKER_OPTIONS} ${PXSCENE_LINK_LIBRARIES})
        add_definitions(${PXSCENE_DEFINITIONS})
        if (PX_LIBRARY_LINK_PXCORE GREATER 0)
            add_dependencies(pxscene_shared pxCore rtCore)
        endif (PX_LIBRARY_LINK_PXCORE GREATER 0)
    endif (BUILD_PXSCENE_SHARED_LIB)
    if (PXWAYLAND_LIBRARY_SUPPORT GREATER 0)
        message("Enabling build support for pxWayland libraries")
        if (BUILD_PXWAYLAND_STATIC_LIB)
            message("Enabling build support for pxWayland static libraries")
            add_library(pxwayland_static ${PXWAYLAND_LIB_FILES})
            set_target_properties(pxwayland_static PROPERTIES OUTPUT_NAME "pxwayland_s")
            target_compile_definitions(pxwayland_static PRIVATE RT_PLATFORM_LINUX PX_NO_WINDOW PX_PLATFORM_GENERIC_EGL ENABLE_PX_WAYLAND_RPC ENABLE_MAX_TEXTURE_SIZE RUNINMAIN ENABLE_HTTP_CACHE BSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE ${PXSCENE_PLATFORM_DEFINE_LIST} PXWAYLANDLIBPH)
        endif (BUILD_PXWAYLAND_STATIC_LIB)

        if (BUILD_PXWAYLAND_SHARED_LIB)
            message("Enabling build support for pxWayland shared libraries")
            add_library(pxwayland_shared SHARED ${PXWAYLAND_LIB_FILES})
            set_target_properties(pxwayland_shared PROPERTIES OUTPUT_NAME "pxwayland")
            target_link_libraries(pxwayland_shared ${LIBRARY_LINKER_OPTIONS} ${PXSCENE_LINK_LIBRARIES})
            target_compile_definitions(pxwayland_shared PRIVATE RT_PLATFORM_LINUX PX_NO_WINDOW PX_PLATFORM_GENERIC_EGL ENABLE_PX_WAYLAND_RPC ENABLE_MAX_TEXTURE_SIZE RUNINMAIN ENABLE_HTTP_CACHE BSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE ${PXSCENE_PLATFORM_DEFINE_LIST} PXWAYLANDLIBPH)
        endif (BUILD_PXWAYLAND_SHARED_LIB)
    endif (PXWAYLAND_LIBRARY_SUPPORT GREATER 0)
else ()
    message("Disabling build support for libraries")
endif (PX_LIBRARY_SUPPORT GREATER 0)

if (BUILD_OPTIMUS_STATIC_LIB)
    message("Enabling Optimus library support")
    add_library(optimus_static optimus_client.cpp)
    add_library(optimus ALIAS optimus_static)
    set_target_properties(optimus_static PROPERTIES OUTPUT_NAME "optimus")
    target_compile_definitions(optimus_static PRIVATE ${PXSCENE_DEFINITIONS})
endif(BUILD_OPTIMUS_STATIC_LIB)
