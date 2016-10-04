all: pxscene2dtests

clean:
	rm -rf $(OBJDIR)
	rm -f pxscene2dtests

SRCS_FULL=\
	pxscene2dtestsmain.cpp \
        exampletests.cpp

MAKEFILE_PATH:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
PXCOREDIR=$(MAKEFILE_PATH)/../../
PX_PLATFORM=PX_PLATFORM_GLUT
PXHDRS=$(PXCOREDIR)/src
EXTDIR=../external
ZLIB=$(EXTDIR)/zlib/
SEARCH=\
	-Ilinux \
	-I$(PXHDRS) \
	-I$(ZLIB) \
	-I./ \
        -I$(PXCOREDIR)examples/pxScene2d/external/googletest-release-1.8.0/googletest/include \
        -I$(PXCOREDIR)examples/pxScene2d/external/googletest-release-1.8.0/googletest \
        -I$(PXCOREDIR)examples/pxScene2d/external/googletest-release-1.8.0/googlemock/include \
        -I$(PXCOREDIR)examples/pxScene2d/external/googletest-release-1.8.0/googlemock

CXXFLAGS= $(SEARCH) -O3 -D$(PX_PLATFORM) -DRT_PLATFORM_LINUX
#-Wall -Wextra -g 
LDPNG    = -L$(EXTDIR)/png/.libs -lpng16
LDFT     = -L$(EXTDIR)/ft/objs/.libs -lfreetype
LDCURL   = -L$(EXTDIR)/curl/lib/.libs -lcurl
LDZLIB   = -L$(EXTDIR)/zlib -lz
LDEXT   = $(LDPNG) $(LDJPG) $(LDFT) $(LDCURL) $(LDZLIB)
LDFLAGS = -L$(PXCOREDIR)/examples/pxScene2d/src/ #-lpxscene 

CXXFLAGS_FULL = $(CXXFLAGS) -Wno-write-strings -Wno-format-security 
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
	$(CC) -c $(CXXFLAGS_FULL) $< -o $@

$(OBJDIR)/gtest-all.o: $(PXCOREDIR)examples/pxScene2d/external/googletest-release-1.8.0/googletest/src/gtest-all.cc
	$(CXX) -o $(OBJDIR)/gtest-all.o -Wall $(SEARCH) $(CFLAGS) -c $(PXCOREDIR)examples/pxScene2d/external/googletest-release-1.8.0/googletest/src/gtest-all.cc

$(OBJDIR)/gmock-all.o: $(PXCOREDIR)examples/pxScene2d/external/googletest-release-1.8.0/googlemock/src/gmock-all.cc 
	$(CXX) -o $(OBJDIR)/gmock-all.o -Wall $(SEARCH) $(CFLAGS) -c $(PXCOREDIR)examples/pxScene2d/external/googletest-release-1.8.0/googlemock/src/gmock-all.cc

pxscene2dtests: $(OBJS) $(OBJDIR)/gtest-all.o $(OBJDIR)/gmock-all.o $(LINKLIBS)
	$(CXX) $(OBJS) $(OBJDIR)/gtest-all.o $(OBJDIR)/gmock-all.o $(LDFLAGS_FULL) -o pxscene2dtests
