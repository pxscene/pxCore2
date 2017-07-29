UNAME_S := $(shell uname -s)
all: pxscene2dtests

clean:
	rm -rf $(OBJDIR)
	rm -f pxscene2dtests

ifeq ($(UNAME_S),Darwin)
SRCS_WAYLAND=
else
SRCS_WAYLAND=\
    test_pxWayland.cpp \
    test_pxWaylandContainer.cpp
endif

SRCS_FULL=\
    pxscene2dtestsmain.cpp \
    test_example.cpp \
    test_api.cpp \
    test_pxcontext.cpp \
    test_memoryleak.cpp \
    test_rtnode.cpp \
    test_pxAnimate.cpp \
    test_jsfiles.cpp \
    test_rtFile.cpp \
    test_rtZip.cpp \
    test_rtString.cpp \
    test_pxImage.cpp \
    test_unzip.cpp \
    $(SRCS_WAYLAND)

ifneq ($(UNAME_S),Darwin)
SRCS_FULL+= test_pxUtil.cpp \
    test_imagecache.cpp \
    test_rtError.cpp
endif

MAKEFILE_PATH:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PXCOREDIR=$(MAKEFILE_PATH)/../../
PXSCENEDIR=$(PXCOREDIR)/examples/pxScene2d
ifeq ($(UNAME_S),Darwin)
PX_PLATFORM=PX_PLATFORM_MAC
else
PX_PLATFORM=PX_PLATFORM_GLUT
endif
PXHDRS=$(PXCOREDIR)/src
EXTDIR=$(PXCOREDIR)examples/pxScene2d/external

ZLIB=$(EXTDIR)/zlib/
PNG=$(EXTDIR)/png/
SEARCH=\
	-Ilinux \
	-I$(PXHDRS) \
	-I$(ZLIB) \
	-I$(PNG) \
	-I$(NODEDIR)/src \
	-I$(NODEDIR)/deps/v8 \
	-I$(NODEDIR)/deps/v8/include \
        -I$(NODEDIR)/deps/uv/include \
	-I./ \
        -I$(PXCOREDIR)examples/pxScene2d/src \
        -I$(EXTDIR)/gtest/googletest/include \
        -I$(EXTDIR)/gtest/googletest \
        -I$(EXTDIR)/gtest/googlemock/include \
        -I$(EXTDIR)/gtest/googlemock \
        -I$(EXTDIR)/curl/include \
        -I$(EXTDIR)/westeros-stub

CXXFLAGS= $(SEARCH) -O3 -Werror -D$(PX_PLATFORM) -DRT_PLATFORM_LINUX -DENABLE_NODE_V_6_9 -DNODE_PLATFORM="linux" -DNODE_WANT_INTERNALS=1 -DV8_DEPRECATION_WARNINGS=1 -DNODE_SHARED_MODE -DNODE_USE_V8_PLATFORM=1 -DNODE_HAVE_I18N_SUPPORT=1 -DNODE_HAVE_SMALL_ICU=1 -DHAVE_INSPECTOR=1 -DV8_INSPECTOR_USE_STL=1 -DV8_INSPECTOR_USE_OLD_STL=1 -DENABLE_HTTP_CACHE  -DENABLE_DEBUG_MODE -DENABLE_LIBJPEG_TURBO -DPNG_APNG_SUPPORTED

#-Wall -Wextra -g
LDPNG    = -L$(EXTDIR)/png/.libs -lpng16
LDFT     = -L$(EXTDIR)/ft/objs/.libs -lfreetype
LDCURL   = -L$(EXTDIR)/curl/lib/.libs -lcurl
LDZLIB   = -L$(EXTDIR)/zlib -lz
ifeq ($(UNAME_S),Darwin)
LDJPG    = $(EXTDIR)/jpg/.libs/libjpeg.a
else
LDJPG    = -L$(EXTDIR)/jpg/.libs -ljpeg
endif
ifeq ($(UNAME_S),Linux)
LDLIBJPEGTURBO = -L$(EXTDIR)/libjpeg-turbo/.libs/ -lturbojpeg
endif
LDEXT   = $(LDPNG) $(LDJPG) $(LDFT) $(LDCURL) $(LDZLIB) $(LDJPG) $(LDLIBJPEGTURBO)
NODEDIR = $(EXTDIR)/libnode-v6.9.0
LDNODE   = -L$(NODEDIR)/out/Release/obj.target
LDNODEV8 = -L$(NODEDIR)/out/Release
ifeq ($(UNAME_S),Darwin)
PXCOREBUILDDIR=-L../../build/mac
LDPLATFORM=-framework OpenGL -framework Cocoa -framework Foundation
else
PXCOREBUILDDIR=-L../../build/glut
LDPLATFORM=-pthread -lglut -lGLEW -lGL -lX11 -lrtCore_s
endif
LDPXSCENE=$(PXCOREDIR)examples/pxScene2d/src/libpxscene.a
LDFLAGS = $(LDPXSCENE) $(LDNODE) $(LDNODEV8) -lnode $(PXCOREBUILDDIR) -lpxCore $(LDEXT) $(LDPLATFORM) -ldl
ifeq ($(CODE_COVERAGE), 1)
CXXFLAGS += -DENABLE_CODE_COVERAGE
ifeq ($(UNAME_S),Darwin)
LLVM_VERSION := $(shell clang --version|grep "LLVM version"|awk '{print $$4}')
LDFLAGS += -lclang_rt.profile_osx -L/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/$(LLVM_VERSION)/lib/darwin
else
LDFLAGS +=  -lgcov
endif
endif
CXXFLAGS_FULL = $(CXXFLAGS) -Wno-write-strings -Wno-format-security -std=c++11
ifeq ($(CODE_COVERAGE), 1)
CXXFLAGS_FULL += -fprofile-arcs -ftest-coverage
endif
CLAGS_FULL = $(CFLAGS)

LDFLAGS_FULL = $(LDFLAGS) -lpthread
OBJDIR=obj
OBJS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRCS_FULL)))
OBJS:=$(patsubst %.c, $(OBJDIR)/%.o, $(OBJS))
OBJS: $(SRCS_FULL)

$(OBJDIR)/%.o : %.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CXX) -c $(CXXFLAGS_FULL) $< -o $@

$(OBJDIR)/%.o : %.c
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) -c $(CLAGS_FULL) $< -o $@

$(OBJDIR)/gtest-all.o: $(EXTDIR)/gtest/googletest/src/gtest-all.cc
	$(CXX) -o $(OBJDIR)/gtest-all.o -Wall $(SEARCH) $(CFLAGS) -c $(EXTDIR)/gtest/googletest/src/gtest-all.cc

$(OBJDIR)/gmock-all.o: $(EXTDIR)/gtest/googlemock/src/gmock-all.cc
	$(CXX) -o $(OBJDIR)/gmock-all.o -Wall $(SEARCH) $(CFLAGS) -c $(EXTDIR)/gtest/googlemock/src/gmock-all.cc

pxscene2dtests: $(OBJS) $(OBJDIR)/gtest-all.o $(OBJDIR)/gmock-all.o $(LINKLIBS)
	$(CXX) $(OBJS) $(OBJDIR)/gtest-all.o $(OBJDIR)/gmock-all.o $(LDFLAGS_FULL) -o pxscene2dtests
