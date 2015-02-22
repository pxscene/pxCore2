
all: gl2d
clean:
	rm -rf $(OBJDIR)
	rm -f gl2d

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
    pxImage.cpp \
    pxUtil.cpp \
    pxContextGL.cpp \
    pxImage9.cpp \
    pxFileDownloader.cpp \
    pxInterpolators.cpp

EXTDIR=../external
PXCOREDIR=../../..
PXHDRS=$(PXCOREDIR)/src
PXLIBS=$(PXCOREDIR)/build/x11

SEARCH=\
  -Ilinux \
  -I$(EXTDIR)/jpg \
  -I$(EXTDIR)/curl/include \
  -I$(EXTDIR)/ft/include \
  -I$(EXTDIR)/png \
  -I$(PXHDRS)

CXXFLAGS=-Wall -Wextra -g $(SEARCH) -DENABLE_GLUT -DRT_PLATFORM_LINUX -DPX_PLATFORM_X11

LDPNG   = -L$(EXTDIR)/png/.libs -lpng16
LDJPG   = -L$(EXTDIR)/jpg/.libs -ljpeg
LDFT    = -L$(EXTDIR)/ft/objs/.libs -lfreetype
LDCURL  = -L$(EXTDIR)/curl/lib/.libs -lcurl
LDEXT   = $(LDPNG) $(LDJPG) $(LDFT) $(LDCURL)
LDFLAGS = -L$(PXLIBS) -lpxCore -pthread -lglut -lGL -lGLEW $(LDEXT) -ldl

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

gl2d: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o gl2d

