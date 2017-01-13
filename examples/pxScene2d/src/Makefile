
UNAME_S := $(shell uname -s)
HNAME_S := $(shell hostname)

ifeq ($(UNAME_S),Darwin)
all: pxscene pxscene.app
deploy: pxscene.dmg
else
all: pxscene
dfb: pxscene-dfb
libs: libpxscene.so
static-libs: libpxscene.a
static-libs-dfb: libpxscene.a-dfb
libs-glut: libpxscene-glut
libs-dfbwindow: libpxscene-dfbwindow
endif

HAVE_WESTEROS := $(shell test -f ../external/westeros/Makefile.ubuntu && echo 1)

ifneq ($(HAVE_WESTEROS),1)
HAVE_WESTEROS := $(shell pkg-config --exists westeros-compositor && echo 1)
endif

clean:
	$(MAKE) -C rpc/ clean
	rm -rf $(OBJDIR)
	rm -f lib*.so*
	rm -f lib*.a
	rm -f pxscene
	rm -rf pxscene.app

ifeq ($(HNAME_S),raspberrypi )
PX_PLATFORM=PX_PLATFORM_GENERIC_EGL
PXLIBS=$(PXCOREDIR)/build/egl
TARGETINCLUDE= -I/opt/vc/include \
  -I/opt/vc/include/interface/vcos/pthreads \
  -I/opt/vc/include/interface/vmcs_host/linux
else
ifeq ($(UNAME_S),Darwin)
PX_PLATFORM=PX_PLATFORM_MAC
PXLIBS=$(PXCOREDIR)/build/mac
TARGETINCLUDE=
else
ifeq ($(PX_PLATFORM),DirectFB)
PX_PLATFORM=PX_PLATFORM_GENERIC_DFB
PXLIBS=$(PXCOREDIR)/build/dfb
else
PX_PLATFORM=PX_PLATFORM_GLUT
PXLIBS=$(PXCOREDIR)/build/glut
TARGETINCLUDE=
endif
endif
endif

ifeq ($(PX_PLATFORM),PX_PLATFORM_GENERIC_DFB)
EXTRA_COMPILER_FLAGS=-std=c++0x
else
EXTRA_COMPILER_FLAGS=-std=c++11
endif


VPATH=linux
RT_SRCS_FULL=\
    utf8.c\
    ioapi_mem.c\
    unzip.c\
    ioapi.c\
    rtError.cpp \
    rtLog.cpp \
    rtFile.cpp \
    rtPathUtils.cpp \
    rtUrlUtils.cpp \
    rtString.cpp \
    rtValue.cpp \
    rtObject.cpp \
    jsCallback.cpp \
    rtFunctionWrapper.cpp \
    rtObjectWrapper.cpp \
    rtThreadPool.cpp \
    rtThreadTask.cpp \
    rtWrapperUtils.cpp \
    rtThreadQueue.cpp \
    rtLibrary.cpp \
    rtZip.cpp \
    linux/rtMutexNative.cpp \
    linux/rtThreadPoolNative.cpp

ifeq ($(HAVE_WESTEROS),1)
else
RT_SRCS_FULL += ../external/westeros-stub/westeros-stub.cpp
endif

PX_SRCS_FULL=\
    pxScene2d.cpp \
    pxResource.cpp \
    pxConstants.cpp \
    pxRectangle.cpp \
    pxFont.cpp \
    pxText.cpp \
    pxTextBox.cpp \
    pxImage.cpp \
    pxUtil.cpp \
    pxImage9.cpp \
    pxFileDownloader.cpp \
    pxInterpolators.cpp \
    pxMatrix4T.cpp \
    pxArchive.cpp \
    pxFileCache.cpp \
    pxHttpCache.cpp 

EXTDIR=../external
PXCOREDIR=../../..
PXHDRS=$(PXCOREDIR)/src
#PXLIBS=$(PXCOREDIR)/build/glut

ifeq ($(PX_PLATFORM),PX_PLATFORM_MAC)
NODEINC=$(EXTDIR)/libnode-v6.9.0
NODEDIR=$(EXTDIR)/libnode-v6.9.0
NODE_V_6_9_DEFINES=-DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1
PLATFORM_DEFINES=
else
ifeq ($(PX_PLATFORM),PX_PLATFORM_GLUT)
NODEINC=$(EXTDIR)/libnode-v6.9.0
NODEDIR=$(EXTDIR)/libnode-v6.9.0
NODE_V_6_9_DEFINES=-DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1
PLATFORM_DEFINES=-DENABLE_LIBJPEG_TURBO
else
NODEINC=$(EXTDIR)/libnode
NODEDIR=$(EXTDIR)/libnode
PLATFORM_DEFINES=
endif
endif

ZLIB=$(EXTDIR)/zlib/


SEARCH=\
  -Ilinux \
  -I$(EXTDIR)/jpg \
  -I$(EXTDIR)/curl/include \
  -I$(EXTDIR)/ft/include \
  -I$(EXTDIR)/png \
  -I$(PXHDRS)\
  -I$(ZLIB) \
  -I./ \
  -I$(NODEINC)/src \
  -I$(NODEINC)/deps/uv/include \
  -I$(NODEINC)/deps/v8 \
  -I$(NODEINC)/deps/debugger-agent/include \
  -I$(NODEINC)/deps/v8/include \
  -I$(NODEINC)/deps/cares/include \
  -I$(EXTDIR)/westeros/external/install/include \
  -I$(EXTDIR)/westeros-stub \
  -I$(EXTDIR)/libjpeg-turbo \
  -Irpc/
   $(TARGETINCLUDE)


#CXXFLAGS=-Wall -Wextra -g $(SEARCH) -DRT_PLATFORM_LINUX -D$(PX_PLATFORM) -DENABLE_RT_NODE
CXXFLAGS2=-fPIC -Wall -Wextra -g $(SEARCH) -DRT_PLATFORM_LINUX -D$(PX_PLATFORM) -DENABLE_RT_NODE -DRUNINMAIN -DENABLE_HTTP_CACHE $(NODE_V_6_9_DEFINES) $(PLATFORM_DEFINES) 

ifdef PXVERSION
CXXFLAGS=$(CXXFLAGS2) -DPX_SCENE_VERSION=$(PXVERSION)
else
CXXFLAGS=$(CXXFLAGS2)
endif

#CXXFLAGS=  $(SEARCH) -DENABLE_GLUT -DRT_PLATFORM_LINUX -DPX_PLATFORM_GLUT
#CXXFLAGS=-Wall -Wextra -g $(SEARCH) -DENABLE_GLUT -DRT_PLATFORM_LINUX -DPX_PLATFORM_GLUT


LDPNG    = -L$(EXTDIR)/png/.libs -lpng16
ifeq ($(UNAME_S),Darwin)
LDJPG    = $(EXTDIR)/jpg/.libs/libjpeg.a
else
LDJPG    = -L$(EXTDIR)/jpg/.libs -ljpeg
endif
LDFT     = -L$(EXTDIR)/ft/objs/.libs -lfreetype
LDCURL   = -L$(EXTDIR)/curl/lib/.libs -lcurl
LDZLIB   = -L$(EXTDIR)/zlib -lz

LDNODE   = -L$(NODEDIR)/out/Release/obj.target
LDNODEV8 = -L$(NODEDIR)/out/Release -L$(NODEDIR)/out/Release/obj.target/deps/v8/tools/gyp

LDLIBJPEGTURBO = -L$(EXTDIR)/libjpeg-turbo/.libs/ -lturbojpeg

#LDNODE   = -L$(NODEDIR)/out/Debug/obj.target
#LDNODEV8 = -L$(NODEDIR)/out/Debug

LDWESTEROS = -L$(EXTDIR)/westeros/external/install/lib -lwesteros_compositor


LDEXT   = $(LDPNG) $(LDJPG) $(LDFT) $(LDCURL) $(LDNODE) $(LDNODEV8) $(LDZLIB)
ifeq ($(HAVE_WESTEROS),1)
LDEXT += $(LDWESTEROS)
endif
ifeq ($(UNAME_S),Linux)
LDEXT += -lX11 $(LDLIBJPEGTURBO)
endif

ifeq ($(UNAME_S),Darwin)
LDFLAGS = -L$(PXLIBS) -lnode -lpxCore -pthread $(LDEXT) -ldl -lv8_libplatform -framework OpenGL -framework Cocoa -framework Foundation
else
LDFLAGS = -L$(PXLIBS) -lnode -lpxCore -pthread -lglut -lGLEW -lGL $(LDEXT) -ldl -lrt -lv8_libplatform 
endif

PLATFORM_LIBS = -lnexus -lnxclient 
ifdef PLATFORM_LIBS_OVERRIDE
PLATFORM_LIBS = $(PLATFORM_LIBS_OVERRIDE)
endif

CXXFLAGS_FULL = $(CXXFLAGS) -fpermissive
CLAGS_FULL = $(CFLAGS)
LDFLAGS_FULL = $(LDFLAGS)

OBJDIR=obj

# rt core begin
RTCORE_SRCS=\
  ../utf8.c \
  ../rtString.cpp \
  ../rtLog.cpp \
  ../rtValue.cpp \
  ../rtObject.cpp \
  ../rtError.cpp \
  ../ioapi_mem.c

RTCORE_OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(RTCORE_SRCS)))
RTCORE_OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(RTCORE_OBJS)))
RTCORE_OBJS:=$(patsubst   %.c, %.o        , $(notdir $(RTCORE_OBJS)))
RTCORE_OBJS:=$(patsubst        %, $(OBJDIR)/%, $(RTCORE_OBJS))
# rt core end

# pxscene so begin
PXSCENE_LIB_SRCS=\
           rtFile.cpp \
           rtPathUtils.cpp \
           rtThreadPool.cpp \
           rtThreadTask.cpp \
           rtThreadQueue.cpp \
           rtLibrary.cpp \
           rtZip.cpp \
           unzip.c \
           ioapi.c \
	   pxScene2d.cpp \
           pxResource.cpp \
           pxConstants.cpp \
	   pxRectangle.cpp \
           pxText.cpp \
           pxTextBox.cpp \
           pxImage.cpp \
           pxUtil.cpp \
           pxImage9.cpp \
           pxFileDownloader.cpp \
           pxInterpolators.cpp \
	   pxMatrix4T.cpp \
           pxArchive.cpp \
           pxFont.cpp \
           pxFileCache.cpp \
           pxHttpCache.cpp \
           rtNode.cpp \
           rtUrlUtils.cpp \
           linux/rtMutexNative.cpp \
           linux/rtThreadPoolNative.cpp \
           jsbindings/rtWrapperUtils.cpp \
           jsbindings/rtFunctionWrapper.cpp \
           jsbindings/rtObjectWrapper.cpp \
           jsbindings/jsCallback.cpp

ifneq ($(PX_PLATFORM),PX_PLATFORM_GENERIC_DFB)
PXSCENE_LIB_SRCS += pxContextGL.cpp 
ifneq ($(DISABLE_WAYLAND),TRUE)
PXSCENE_LIB_SRCS += pxWayland.cpp 
PXSCENE_LIB_SRCS += pxWaylandContainer.cpp
endif
else
PXSCENE_LIB_SRCS += pxContextDFB.cpp
endif


ifeq ($(HAVE_WESTEROS),1)
else
PXSCENE_LIB_SRCS += ../external/westeros-stub/westeros-stub.cpp
endif

PXSCENE_LIB_OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(PXSCENE_LIB_SRCS)))
PXSCENE_LIB_OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(PXSCENE_LIB_OBJS)))
PXSCENE_LIB_OBJS:=$(patsubst   %.c, %.o        , $(notdir $(PXSCENE_LIB_OBJS)))
PXSCENE_LIB_OBJS:=$(patsubst        %, $(OBJDIR)/%, $(PXSCENE_LIB_OBJS))

# pxWayland so begin
PXWAYLAND_LIB_SRCS=\
            linux/rtMutexNative.cpp \
            pxContextGL.cpp \
            rtThreadQueue.cpp \
            pxUtil.cpp \
            rtFile.cpp

ifneq ($(DISABLE_WAYLAND),TRUE)
PXWAYLAND_LIB_SRCS += pxWayland.cpp
endif

PXWAYLAND_LIB_OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(PXWAYLAND_LIB_SRCS)))
PXWAYLAND_LIB_OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(PXWAYLAND_LIB_OBJS)))
PXWAYLAND_LIB_OBJS:=$(patsubst   %.c, %.o        , $(notdir $(PXWAYLAND_LIB_OBJS)))
PXWAYLAND_LIB_OBJS:=$(patsubst        %, $(OBJDIR)/%, $(PXWAYLAND_LIB_OBJS))

PXSCENE_LIB_SEARCH=\
  -Ilinux \
  -I$(PXHDRS)\
  -I./ \
  -I$(NODEINC)/src \
  -I$(NODEINC)/deps/uv/include \
  -I$(NODEINC)/deps/v8 \
  -I$(NODEINC)/deps/debugger-agent/include \
  -I$(NODEINC)/deps/v8/include \
  -I$(NODEINC)/deps/cares/include \
  -Irpc \
  $(PXCORE_INCLUDES)

# pxScene and pxWayland so end

rtMutexNative.o: linux/rtMutexNative.cpp
	$(CXX) -o $(OBJDIR)/rtMutexNative.o -Wall $(CXXFLAGS_FULL) $(EXTRA_COMPILER_FLAGS) -c linux/rtMutexNative.cpp

rtThreadPoolNative.o: linux/rtThreadPoolNative.cpp
	$(CXX) -o $(OBJDIR)/rtThreadPoolNative.o -Wall $(CXXFLAGS_FULL) $(EXTRA_COMPILER_FLAGS) -c linux/rtThreadPoolNative.cpp

$(OBJDIR)/rtWrapperUtils.o: jsbindings/rtWrapperUtils.cpp
	$(CXX) -o $(OBJDIR)/rtWrapperUtils.o -Wall $(CXXFLAGS_FULL) $(EXTRA_COMPILER_FLAGS) -c jsbindings/rtWrapperUtils.cpp

$(OBJDIR)/rtFunctionWrapper.o: jsbindings/rtFunctionWrapper.cpp
	$(CXX) -o $(OBJDIR)/rtFunctionWrapper.o -Wall $(CXXFLAGS_FULL) $(EXTRA_COMPILER_FLAGS) -c jsbindings/rtFunctionWrapper.cpp

$(OBJDIR)/rtObjectWrapper.o: jsbindings/rtObjectWrapper.cpp
	$(CXX) -o $(OBJDIR)/rtObjectWrapper.o -Wall $(CXXFLAGS_FULL) $(EXTRA_COMPILER_FLAGS) -c jsbindings/rtObjectWrapper.cpp

$(OBJDIR)/jsCallback.o: jsbindings/jsCallback.cpp
	$(CXX) -o $(OBJDIR)/jsCallback.o -Wall $(CXXFLAGS_FULL) $(EXTRA_COMPILER_FLAGS) -c jsbindings/jsCallback.cpp

$(OBJDIR)/westeros-stub.o: $(EXTDIR)/westeros-stub/westeros-stub.cpp
	$(CXX) -o $(OBJDIR)/westeros-stub.o -Wall $(CXXFLAGS_FULL) $(EXTRA_COMPILER_FLAGS) -c $(EXTDIR)/westeros-stub/westeros-stub.cpp

SRCS_FULL_GL=$(PX_SRCS_FULL) $(RT_SRCS_FULL) pxContextGL.cpp pxWayland.cpp pxWaylandContainer.cpp rtNode.cpp pxScene.cpp
SRCS_FULL_DFB=$(PX_SRCS_FULL) $(RT_SRCS_FULL) pxContextDFB.cpp rtNode.cpp pxScene.cpp

OBJS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRCS_FULL_GL)))
OBJS:=$(patsubst %.c, $(OBJDIR)/%.o, $(OBJS))
OBJS: $(SRCS_FULL_GL)

OBJS_DFB=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRCS_FULL_DFB)))
OBJS_DFB:=$(patsubst %.c, $(OBJDIR)/%.o, $(OBJS_DFB))
OBJS_DFB: $(SRCS_FULL_DFB)

$(OBJDIR)/%.o : %.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CXX) -c $(CXXFLAGS_FULL) $(EXTRA_COMPILER_FLAGS) $< -o $@

$(OBJDIR)/%.o : %.c
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) -c $(CXXFLAGS_FULL) $< -o $@


# Enable these. There are already a bunch of things
# enabled by default. These are in addition to those.
SCAN_CHECKS=\
  alpha.core.BoolAssignment\
  alpha.core.CastSize\
  alpha.core.FixedAddr\
  alpha.core.SizeofPtr\
  alpha.core.TestAfterDivZero\
  alpha.cplusplus.NewDeleteLeaks\
  alpha.cplusplus.VirtualCall\
  alpha.security.ArrayBoundV2\
  alpha.security.MallocOverflow\
  alpha.security.ReturnPtrRange\
  alpha.unix.MallocWithAnnotations\
  alpha.unix.PthreadLock\
  alpha.unix.SimpleStream\
  alpha.unix.cstring.NotNullTerminated\
  alpha.unix.cstring.OutOfBounds\
  security.insecureAPI.strcpy

LLVM_CHECKS=$(patsubst %, -enable-checker %, $(SCAN_CHECKS))

LIBRTCORE:=rtCore
LIBRTCORE_TARGET:=lib$(LIBRTCORE).so
LIBRTCORE_TARGET_S:=lib$(LIBRTCORE)_s.a
LIBRTREMOTE:=rtRemote
LIBRTREMOTE_TARGET:=lib$(LIBRTREMOTE).so
LIBRTREMOTE_TARGET_S:=lib$(LIBRTREMOTE)_s.a

ifeq ($(PX_PLATFORM),PX_PLATFORM_GENERIC_DFB)
PXSCENE_LIBS=-lpng15 -ldirectfb -ldirect -lfusion
PXSCENE_LIB_LINK_OPTIONS=$(PXSCENE_LIB_LINKING) -L$(PXLIBS)  -Wl,--whole-archive $(PXLIBS)/libpxCore.a
PXSCENE_LIB_DEFINES=-DPX_PLATFORM_GENERIC_DFB -DENABLE_DFB -DENABLE_DFB_GENERIC
LIBRTREMOTE=
LIBRTREMOTE_TARGET=
LIBRTREMOTE_TARGET_S=
PXSCENE_EXTRA_DEFINES=-DNEXUS_SERVER_SUPPORT=1 -D_GNU_SOURCE=1 -DLINUX -pipe -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE -mips32 -DNEXUS_PLATFORM_VERSION_MAJOR=14 -DNEXUS_PLATFORM_VERSION_MINOR=4 -DNEXUS_PLATFORM_VERSION_NUMBER=3588 -DNEXUS_CONFIG_IMAGE -DNEXUS_MODE_proxy -DPLATFORM=97125 -DNEXUS_PLATFORM=97125 -DNEXUS_HAS_MPOD -DBCHP_CHIP=7125 -DBCHP_VER=BCHP_VER_C0 -DBMEM_REENTRANT_CONFIG=BMEM_REENTRANT -DBINT_REENTRANT_CONFIG=BINT_REENTRANT -DNEXUS_PROFILE_OS_linuxuser -DBBCP_SUPPORT -DNEXUS_FRONTEND_3412 -DNEXUS_FRONTEND_3255 -DNEXUS_FRONTEND_ACCUMULATE_STATISTICS=1 -DNEXUS_FRONTEND_7125 -DNEXUS_AMPLIFIER_SUPPORT -DNEXUS_TUNER_SUPPORT -DNEXUS_FRONTEND_3112 -DNEXUS_FRONTEND_3109 -DNEXUS_FRONTEND_31xx_B0 -DNEXUS_SECURITY_SC_VALUE -DNEXUS_SECURITY_EXT_KEY_IV -DNEXUS_SECURITY_SC_VALUE -DNEXUS_SECURITY_EXT_KEY_IV -DNEXUS_SECURITY_HAS_ASKM=1 -DNEXUS_SECURITY_CHIP_SIZE=65 -DNEXUS_KEYLADDER=1 -DNEXUS_HAS_KEYLADDER_SUPPORT=1 -DNEXUS_OTPMSP=1 -DNEXUS_OTPID=1 -DNEXUS_USERCMD=1 -DNEXUS_SECURITY_SECUREACCESS=1 -DNEXUS_SECURITY_SECURERSA=1 -DBXPT_MESG_DONT_AUTO_ENABLE_PID_CHANNEL -DB_REFSW_DSS_SUPPORT=1 -DNEXUS_OTFPVR=1 -DB_REFSW_DSS_SUPPORT=1 -DNEXUS_AC3 -DNEXUS_DDP -DNEXUS_AC3 -DMACROVISION_SUPPORT=1 -DNEXUS_HAS_PLATFORM -DNEXUS_HAS_CORE -DNEXUS_HAS_PWM -DNEXUS_HAS_I2C -DNEXUS_HAS_GPIO -DNEXUS_HAS_LED -DNEXUS_HAS_IR_INPUT -DNEXUS_HAS_IR_BLASTER -DNEXUS_HAS_INPUT_CAPTURE -DNEXUS_HAS_KEYPAD -DNEXUS_HAS_FRONTEND -DNEXUS_HAS_SPI -DNEXUS_HAS_SECURITY -DNEXUS_HAS_DMA -DNEXUS_HAS_TRANSPORT -DNEXUS_HAS_VIDEO_DECODER -DNEXUS_HAS_AUDIO -DNEXUS_HAS_SURFACE -DNEXUS_HAS_ASTM -DNEXUS_HAS_DISPLAY -DNEXUS_HAS_GRAPHICS2D -DNEXUS_HAS_SYNC_CHANNEL -DNEXUS_HAS_HDMI_OUTPUT -DNEXUS_HAS_CEC -DNEXUS_HAS_SMARTCARD -DNEXUS_HAS_SURFACE_COMPOSITOR -DNEXUS_HAS_INPUT_ROUTER -DNEXUS_HAS_SIMPLE_DECODER -DNEXUS_HAS_FILE -DNEXUS_HAS_PLAYBACK -DNEXUS_HAS_RECORD -DB_REFSW_DEBUG_FORCE_ENABLE_BDBG=1 -O3 -DB_REFSW_DEBUG_COMPACT_ASSERT=1 -DBDBG_NO_WRN=1 -DBDBG_NO_MSG=1 -DB_REFSW_DEBUG_COMPACT_ERR=1 -DB_REFSW_DEBUG_FORCE_ENABLE_BDBG=1 -DRDK_USE_NXCLIENT
else 
PXSCENE_LIBS=-lpng16 -lnxpl -lGLESv2 -Lrpc/ -l$(LIBRTREMOTE) -luuid
ifneq ($(DISABLE_WESTEROS),TRUE)
PXSCENE_LIBS:=-lwesteros_compositor 
endif
PXSCENE_LIB_LINK_OPTIONS=-L$(PXCOREDIR)/build/egl -Wl,--whole-archive $(PXCOREDIR)/build/egl/libpxCore.a
ifeq ($(DISABLE_WAYLAND),TRUE)
PXSCENE_LIB_DEFINES=-DPX_PLATFORM_GENERIC_EGL -DENABLE_PX_WAYLAND_RPC -DENABLE_MAX_TEXTURE_SIZE -DDISABLE_WAYLAND
else
PXSCENE_LIB_DEFINES=-DPX_PLATFORM_GENERIC_EGL -DENABLE_PX_WAYLAND_RPC -DENABLE_MAX_TEXTURE_SIZE
endif
PXSCENE_EXTRA_DEFINES=
endif


analyze:
	$(MAKE) clean
	scan-build-3.5 $(LLVM_CHECKS) $(MAKE) pxscene

LINKLIBS := $(PXLIBS)/libpxCore.a
pxscene: $(OBJS) $(LINKLIBS)
	$(CXX) $(OBJS) $(LDFLAGS_FULL) -o pxscene

INC_DFB=-I/usr/local/include/directfb

pxscene-dfb: CXXFLAGS_FULL = -fPIC -Wall -Wextra -g $(SEARCH) $(INC_DFB) -DRT_PLATFORM_LINUX -DPX_PLATFORM_X11 -DENABLE_RT_NODE -DRUNINMAIN -DENABLE_HTTP_CACHE -DENABLE_DFB 
pxscene-dfb: $(OBJS_DFB) $(PXCOREDIR)/build/dfb/libpxCore.a 
	$(CXX) $(OBJS_DFB) -lnode -lpxCore -pthread -L/usr/local/lib -ldirectfb $(LDEXT) -L$(PXCOREDIR)/build/dfb -lpxCore -ldl -lrt -lv8_libplatform -o pxscene

librtRemote.so:
	$(MAKE) -C rpc/ librtRemote.so

librtRemote_s.a:
	$(MAKE) -C rpc/ librtRemote_s.a

cleansceneobj:
	rm -f $(PXSCENE_LIB_OBJS)

cleanwaylandobj:
	rm -f $(PXWAYLAND_LIB_OBJS)

libpxscene-glut:
	rm -rf $(OBJDIR)
libpxscene-glut: CFLAGS_FULL = $(CFLAGS)
#libpxscene-glut: CXXFLAGS_FULL =-Wno-attributes -Wall -Wextra -fpermissive -fPIC -g -DRT_PLATFORM_LINUX -DPX_PLATFORM_GLUT -DENABLE_RT_NODE -DRUNINMAIN -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE $(PXSCENE_EXTRA_DEFINES)
#libpxscene-glut: LDFLAGS_FULL = $(LDEXT) -Wl,--no-whole-archive -lglut -lGLEW -lGL -lnode -pthread -ljpeg -lfreetype -lcurl $(LDNODE) $(LDNODEV8) -Lrpc/ -ldl -lrt -L./ -l$(LIBRTCORE) 
libpxscene-glut: cleansceneobj $(PXSCENE_LIB_OBJS) $(LIBRTCORE_TARGET) 
	$(AR) rc libpxscene.a $(PXSCENE_LIB_OBJS) 

libpxscene-dfbwindow:
	rm -rf $(OBJDIR)
libpxscene-dfbwindow: CFLAGS_FULL = $(CFLAGS)
libpxscene-dfbwindow: CXXFLAGS_FULL =-Wno-attributes -Wall -Wextra -fpermissive -fPIC $(PXSCENE_LIB_SEARCH) -g -DRT_PLATFORM_LINUX -DENABLE_DFB -DPX_PLATFORM_DFB_NON_X11 -DENABLE_RT_NODE -DENABLE_V8_HEAP_PARAMS -DRUNINMAIN -DENABLE_HTTP_CACHE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE $(PXSCENE_EXTRA_DEFINES) -std=c++0x
libpxscene-dfbwindow: LDFLAGS_FULL =$(PXSCENE_LIB_LINK_OPTIONS) -Wl,--no-whole-archive -L../../../../workRNG150/rootfs/usr/local/lib/ -lnode -pthread -ljpeg -lfreetype -lcurl $(PXSCENE_LIBS) $(LDNODE) $(LDNODEV8) -Lrpc/ -ldl -lrt -L./ -l$(LIBRTCORE) $(PLATFORM_LIBS) -lpng15 -ldirectfb -ldirect -lfusion -lz -lssl -lcrypto -shared -o libpxscene.so.1.0
libpxscene-dfbwindow: cleansceneobj $(PXSCENE_LIB_OBJS) $(LIBRTCORE_TARGET) $(LIBRTREMOTE_TARGET)
	$(CXX) $(PXSCENE_LIB_OBJS) $(LDFLAGS_FULL)
	ln -sf libpxscene.so.1.0 libpxscene.so.1
	ln -sf libpxscene.so.1 libpxscene.so

pxscene-with-so-dfb: CFLAGS_FULL = $(CFLAGS)
pxscene-with-so-dfb: CXXFLAGS_FULL =-Wno-attributes -Wall -Wextra -fpermissive -fPIC $(PXSCENE_LIB_SEARCH) -g -DRT_PLATFORM_LINUX -DENABLE_DFB -DPX_PLATFORM_DFB_NON_X11 -DENABLE_RT_NODE -DENABLE_V8_HEAP_PARAMS -DRUNINMAIN -DENABLE_HTTP_CACHE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE $(PXSCENE_EXTRA_DEFINES) -std=c++0x
pxscene-with-so-dfb: LDFLAGS_FULL =$(PXSCENE_LIB_LINK_OPTIONS) -Wl,--no-whole-archive -L../../../../workRNG150/rootfs/usr/local/lib/ -lnode -pthread -ljpeg -lfreetype -lcurl $(PXSCENE_LIBS) $(LDNODE) $(LDNODEV8) -Lrpc/ -ldl -lrt -L./ -l$(LIBRTCORE) $(PLATFORM_LIBS) -lpng15 -ldirectfb -ldirect -lfusion -lz -lssl -lcrypto -lpxscene -o pxscene
pxscene-with-so-dfb: cleansceneobj $(PXSCENE_LIB_OBJS) $(LIBRTCORE_TARGET) $(LIBRTREMOTE_TARGET)
	$(CXX) $(CXXFLAGS_FULL) pxScene.cpp $(LDFLAGS_FULL)

 

libpxscene.so-node-v6.9: PXSCENE_VERSION_DEFINES = $(PXSCENE_EXTRA_DEFINES) -DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1
libpxscene.so-node-v6.9: libpxscene.so

libpxscene.so:
	rm -rf $(OBJDIR)
libpxscene.so: CFLAGS_FULL = $(CFLAGS)
libpxscene.so: CXXFLAGS_FULL =-Wno-attributes -Wall -Wextra -fpermissive -fPIC $(PXSCENE_LIB_SEARCH) $(EXTRA_COMPILER_FLAGS) -g -DRT_PLATFORM_LINUX -DPX_NO_WINDOW $(PXSCENE_LIB_DEFINES) -DENABLE_RT_NODE -DENABLE_V8_HEAP_PARAMS -DENABLE_PX_SCENE_TEXTURE_USAGE_MONITORING -DRUNINMAIN -DENABLE_HTTP_CACHE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE $(PXSCENE_EXTRA_DEFINES) $(PXSCENE_VERSION_DEFINES)
libpxscene.so: LDFLAGS_FULL =$(PXSCENE_LIB_LINK_OPTIONS) -Wl,--no-whole-archive -lnode -pthread -ljpeg -lfreetype -lcurl $(PXSCENE_LIBS) $(LDNODE) $(LDNODEV8) -Lrpc/ -ldl -lrt -L./ -l$(LIBRTCORE) -shared $(PLATFORM_LIBS) -o libpxscene.so.1.0  
libpxscene.so: cleansceneobj $(PXSCENE_LIB_OBJS) $(LIBRTCORE_TARGET) $(LIBRTREMOTE_TARGET)
	$(CXX) $(PXSCENE_LIB_OBJS) $(LDFLAGS_FULL)
	ln -sf libpxscene.so.1.0 libpxscene.so.1
	ln -sf libpxscene.so.1 libpxscene.so

libpxscene.a:
	rm -rf $(OBJDIR)
libpxscene.a: CFLAGS_FULL = $(CFLAGS)
libpxscene.a: PXSCENE_VERSION_DEFINES = $(PXSCENE_EXTRA_DEFINES) -DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1
libpxscene.a: CXXFLAGS_FULL =-Wno-attributes -Wall -Wextra -fpermissive -fPIC $(PXSCENE_LIB_SEARCH) $(EXTRA_COMPILER_FLAGS) -g -DRT_PLATFORM_LINUX -DPX_NO_WINDOW $(PXSCENE_LIB_DEFINES) -DENABLE_RT_NODE -DENABLE_V8_HEAP_PARAMS -DENABLE_PX_SCENE_TEXTURE_USAGE_MONITORING -DRUNINMAIN -DENABLE_HTTP_CACHE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE $(PXSCENE_EXTRA_DEFINES) $(PXSCENE_VERSION_DEFINES)
libpxscene.a: cleansceneobj $(PXSCENE_LIB_OBJS) $(LIBRTCORE_TARGET) $(LIBRTREMOTE_TARGET)
	$(AR) rcs -o $@ $(PXSCENE_LIB_OBJS)

libpxscene.a-dfb:
	rm -rf $(OBJDIR)
libpxscene.a-dfb: CFLAGS_FULL = $(CFLAGS)
libpxscene.a-dfb: CXXFLAGS_FULL =-Wno-attributes -Wall -Wextra -fpermissive -fPIC $(PXSCENE_LIB_SEARCH) $(EXTRA_COMPILER_FLAGS) -g -DRT_PLATFORM_LINUX -DPX_NO_WINDOW $(PXSCENE_LIB_DEFINES) -DENABLE_RT_NODE -DENABLE_V8_HEAP_PARAMS -DENABLE_PX_SCENE_TEXTURE_USAGE_MONITORING -DRUNINMAIN -DENABLE_HTTP_CACHE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE $(PXSCENE_EXTRA_DEFINES) $(PXSCENE_VERSION_DEFINES)
libpxscene.a-dfb: LDFLAGS_FULL =$(PXSCENE_LIB_LINK_OPTIONS) -Wl,--no-whole-archive -pthread -ljpeg -lfreetype -lcurl $(PXSCENE_LIBS) $(LDNODE) $(LDNODEV8) -Lrpc/ -ldl -lrt -L./ -l$(LIBRTCORE) -shared $(PLATFORM_LIBS)  
libpxscene.a-dfb: cleansceneobj $(PXSCENE_LIB_OBJS) $(LIBRTCORE_TARGET) $(LIBRTREMOTE_TARGET)
	$(AR) rcs -o libpxscene.a $(PXSCENE_LIB_OBJS)

libpxwayland.so: 
	rm -rf $(OBJDIR)
libpxwayland.so: CFLAGS_FULL = $(CFLAGS)
libpxwayland.so: CXXFLAGS_FULL =-Wno-attributes -Wall -Wextra -fpermissive -fPIC $(PXSCENE_LIB_SEARCH) $(EXTRA_COMPILER_FLAGS) -g -DRT_PLATFORM_LINUX -DPX_NO_WINDOW $(PXSCENE_LIB_DEFINES) -DRUNINMAIN -DENABLE_HTTP_CACHE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE
libpxwayland.so: LDFLAGS_FULL =$(PXSCENE_LIB_LINK_OPTIONS) -Wl,--no-whole-archive -pthread -ljpeg -lfreetype -lcurl $(PXSCENE_LIBS) -Lrpc/ -ldl -lrt -L./ -l$(LIBRTCORE) -shared $(PLATFORM_LIBS) -o libpxwayland.so.1.0
libpxwayland.so: cleanwaylandobj $(PXWAYLAND_LIB_OBJS) $(LIBRTCORE_TARGET) $(LIBRTREMOTE_TARGET)
	$(CXX) $(PXWAYLAND_LIB_OBJS) $(LDFLAGS_FULL)
	ln -sf libpxwayland.so.1.0 libpxwayland.so.1
	ln -sf libpxwayland.so.1 libpxwayland.so

libpxwayland_s.a: CFLAGS_FULL = $(CFLAGS)
libpxwayland_s.a: CXXFLAGS_FULL =-Wno-attributes -Wall -Wextra -fpermissive -fPIC $(PXSCENE_LIB_SEARCH) $(EXTRA_COMPILER_FLAGS) -g -DRT_PLATFORM_LINUX -DPX_NO_WINDOW $(PXSCENE_LIB_DEFINES) -DRUNINMAIN -DENABLE_HTTP_CACHE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE
libpxwayland_s.a: cleanwaylandobj $(PXWAYLAND_LIB_OBJS) $(LIBRTCORE_TARGET_S) $(LIBRTREMOTE_TARGET_S)
	$(AR) rcs -o $@ $(PXWAYLAND_LIB_OBJS)

librtCore.so: CFLAGS_FULL = $(CFLAGS) -DRAPIDJSON_HAS_STDSTRING -Wall -Wextra -DRT_PLATFORM_LINUX -I../ -I. -fPIC
librtCore.so: CXXFLAGS_FULL = $(CXXFLAGS) -std=c++0x $(CFLAGS_FULL)
librtCore.so: LDFLAGS_FULL = -pthread -ldl -shared -o $@
librtCore.so: $(RTCORE_OBJS)
	$(CXX) $(RTCORE_OBJS) $(LDFLAGS_FULL)

librtCore_s.a: CFLAGS_FULL = $(CFLAGS) -DRAPIDJSON_HAS_STDSTRING -Wall -Wextra -DRT_PLATFORM_LINUX -I../ -I. -fPIC
librtCore_s.a: CXXFLAGS_FULL = $(CXXFLAGS) -std=c++0x $(CFLAGS_FULL)
librtCore_s.a: $(RTCORE_OBJS)
	$(AR) rcs -o $@ $(RTCORE_OBJS)

pxscene.app: pxscene
	rm -rf pxscene.app
	./mkapp.sh

pxscene.dmg:
ifndef PXVERSION
	$(error PXVERSION must be defined for the deploy target)
endif
	make clean
	make PX_SCENE_VERSION2=PXVERSION pxscene.app
	echo $(PXVERSION)
	./mkdeploy.sh $(PXVERSION)

.PHONY: all deploy dfb libs libs-glut libpxscene-glut libpxscene-dfbwindow pxscene-with-so-dfb
.PHONY: analyze clean cleansceneobj cleanwaylandobj
