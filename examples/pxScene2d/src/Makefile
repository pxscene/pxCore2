
all: gl2d
clean:
	rm -rf $(OBJDIR)
	rm -f gl2d

UNAME_S := $(shell uname -s)
HNAME_S := $(shell hostname) 

ifeq ($(HNAME_S),raspberrypi ) 
PX_PLATFORM=PX_PLATFORM_GENERIC_EGL
PXLIBS=$(PXCOREDIR)/build/egl
TARGETINCLUDE= -I/opt/vc/include \
  -I/opt/vc/include/interface/vcos/pthreads \
  -I/opt/vc/include/interface/vmcs_host/linux
else
PX_PLATFORM=PX_PLATFORM_GLUT
PXLIBS=$(PXCOREDIR)/build/glut
TARGETINCLUDE=
endif

VPATH=linux
RT_SRCS=\
    rtError.cpp\
    rtFile.cpp \
    rtLog.cpp \
    rtPathUtils.cpp \
    rtString.cpp \
    rtObject.cpp \
    rtValue.cpp \
    linux/rtMutexNative.cpp \
    linux/rtThreadPoolNative.cpp \
    rtThreadPool.cpp \
    rtThreadTask.cpp \
    rtLibrary.cpp \
    utf8.c
    
PX_SRCS=\
    pxMain.cpp \
    pxScene2d.cpp \
		pxRectangle.cpp \
    pxText.cpp \
    pxText2.cpp \
    pxImage.cpp \
    pxUtil.cpp \
    pxContextGL.cpp \
    pxImage9.cpp \
    pxTextureCacheObject.cpp \
    pxTexture.cpp \
    pxFileDownloader.cpp \
    pxInterpolators.cpp \
		pxMatrix4T.cpp

EXTDIR=../external
PXCOREDIR=../../..
PXHDRS=$(PXCOREDIR)/src

SEARCH=\
  -Ilinux \
  -I$(EXTDIR)/jpg \
  -I$(EXTDIR)/curl/include \
  -I$(EXTDIR)/ft/include \
  -I$(EXTDIR)/png \
  -I$(PXHDRS) \
	$(TARGETINCLUDE)

CXXFLAGS=-Wall -Wextra -g $(SEARCH) -DRT_PLATFORM_LINUX -D$(PX_PLATFORM) 

LDPNG   = -L$(EXTDIR)/png/.libs -lpng16
ifeq ($(UNAME_S),Darwin)
LDJPG    = $(EXTDIR)/jpg/.libs/libjpeg.a
else
LDJPG   = -L$(EXTDIR)/jpg/.libs -ljpeg
endif
LDFT    = -L$(EXTDIR)/ft/objs/.libs -lfreetype
LDCURL  = -L$(EXTDIR)/curl/lib/.libs -lcurl
LDEXT   = $(LDPNG) $(LDJPG) $(LDFT) $(LDCURL)

ifeq ($(UNAME_S),Darwin)
LDFLAGS = -L$(PXLIBS) -lpxCore $(LDEXT) -framework GLUT -framework OpenGL -ldl
else
ifeq ($(HNAME_S),raspberrypi )
LDFLAGS = -L$(PXLIBS) -lpxCore -pthread -lGLESv2 $(LDEXT) -ldl -lEGL -lrt -L/opt/vc/lib -lvcos -lbcm_host
else
LDFLAGS = -L$(PXLIBS) -lpxCore -pthread -lglut -lGL -lGLEW $(LDEXT) -ldl
endif
endif

OBJDIR=obj
SRCS=$(PX_SRCS) $(RT_SRCS) testScene.cpp

OBJS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRCS)))
OBJS:=$(patsubst %.c, $(OBJDIR)/%.o, $(OBJS))
OBJS: $(SRCS)

$(OBJDIR)/%.o : %.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o : %.c
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) -c $(CXXFLAGS) $< -o $@


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

analyze:
	$(MAKE) clean
	scan-build-3.5 $(LLVM_CHECKS) $(MAKE) gl2d

PXMAIN=$(OBJDIR)/pxMain.o
POLY_BUG_OBJS=$(filter-out $(PXMAIN), $(OBJS)) $(OBJDIR)/poly_bug.o
poly_bug: $(POLY_BUG_OBJS)
	$(CXX) $(POLY_BUG_OBJS) $(LDFLAGS) -o poly_bug

gl2d: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o gl2d

