
all: rasterDFB

clean:
	rm -f rasterDFB

########################################################################

# /home/hfitzpatrick/projects/RNG150/master/workRNG150/Refsw/uclinux-rootfs/lib/directfb
LD_DFB=$(ROOTFS)/usr/local/lib
INC_DFB=${DFB_ROOT}/usr/local/include/directfb

########################################################################

VPATH=linux

SRCS_FULL=\
	rasterDFB.cpp \
	pxTimerNative.cpp \
	pxMatrix4T.cpp

SEARCH=\
  -I. \
  -I./x11 \
  $(DFB_INCLUDES)

#CXXFLAGS= $(SEARCH) -O3
#-Wall -Wextra -g

LDFLAGS = $(DFB_LIBS) -ldirectfb -lfusion -ldirect -lz -lnexus -lnxclient

CXXFLAGS= $(SEARCH) -O3 -DPX_PLATFORM_DFB_NON_X11 -DENABLE_DFB -fPIC -DRT_PLATFORM_LINUX -DENABLE_RT_NODE -DBSTD_CPU_ENDIAN=BSTD_ENDIAN_LITTLE

CXXFLAGS_FULL = $(CXXFLAGS) -Wno-write-strings -Wno-format-security
CLAGS_FULL = $(CFLAGS)
LDFLAGS_FULL = $(LDFLAGS)

OBJDIR=obj


OBJS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SRCS_FULL)))
OBJS:=$(patsubst %.c, $(OBJDIR)/%.o, $(OBJS))
OBJS: $(SRCS_FULL_GL)


pxTimerNative.o: pxTimerNative.cpp
	$(CXX) -o pxTimerNative.o -Wall $(INCDIR) $(CFLAGS) -c pxTimerNative.cpp

pxMatrix4T.o: x11/pxMatrix4T.cpp
	$(CXX) -o pxMatrix4T.o -Wall $(INCDIR) $(CFLAGS) -c x11/pxMatrix4T.cpp


$(OBJDIR)/%.o : %.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CXX) -c $(CXXFLAGS_FULL) $< -o $@

$(OBJDIR)/%.o : %.c
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) -c $(CXXFLAGS_FULL) $< -o $@

rasterDFB: $(OBJS) $(LINKLIBS)
	$(CXX) $(OBJS) $(LDFLAGS_FULL) -o rasterDFB

