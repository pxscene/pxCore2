
RTMSG_TARGET=rtMessaging
RTMSG_LIBNAME=lib$(RTMSG_TARGET).so
RTMSGD_TARGET=rtmsgd

all: $(RTMSG_LIBNAME) $(RTMSGD_TARGET)

BUILD_CXX ?= $(CXX)

RTCORE_SRCS=\
  ../utf8.c \
  ../rtString.cpp \
  ../rtLog.cpp \
  ../rtValue.cpp \
  ../rtObject.cpp \
  ../rtError.cpp

RTMSG_SRCS=\
  rtIpAddress.cpp \
  rtSocket.cpp \
  rtBuffer.cpp \
  rtSelector.cpp

RTMSGD_SRCS=\
  rtmsgd.cpp

ifeq ($V, 1)
  CXX_PRETTY = $(CXX)
  LD_PRETTY = $(CXX)
  CC_PRETTY = $(CC)
  BUILD_CXX_PRETTY = $(BUILD_CXX)
else
  CXX_PRETTY = @echo " [CXX] $<" ; $(CXX)
  LD_PRETTY = @echo "[LINK] $@" ; $(CXX)
  CC_PRETTY = @echo " [CC] $<" ; $(CC)
  BUILD_CXX_PRETTY = @echo " [CC] $<" ; $(BUILD_CXX)
endif

ifeq ($(DEBUG), 1)
  CXXFLAGS += -DRT_RPC_DEBUG -DRT_DEBUG
  CFLAGS   += -g -O0 -fno-inline
else
  CXXFLAGS += -O2
  CFLAGS   += -O2
endif

ifeq ($(PROFILE), 1)
  CXXFLAGS += -pg
endif


CFLAGS+=-Wall -Wextra -DRT_PLATFORM_LINUX -I../src -I. -fPIC
CXXFLAGS+=-std=c++0x $(CFLAGS)
LDFLAGS =-pthread -ldl -luuid -Wl,-rpath=../../,--enable-new-dtags
OBJDIR=obj

RTCORE_OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(RTCORE_SRCS)))
RTCORE_OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(RTCORE_OBJS)))
RTCORE_OBJS:=$(patsubst   %.c, %.o        , $(notdir $(RTCORE_OBJS)))
RTCORE_OBJS:=$(patsubst        %, $(OBJDIR)/%, $(RTCORE_OBJS))

RTMSG_OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(RTMSG_SRCS)))
RTMSG_OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(RTMSG_OBJS)))
RTMSG_OBJS:=$(patsubst        %, $(OBJDIR)/%, $(RTMSG_OBJS))

RTMSGD_OBJS=$(patsubst %.cpp, %.o, $(notdir $(RTMSGD_SRCS)))
$(RTMSGD_OBJS): $(CXXFLAGS)+=-I../src

SAMPLEAPP_OBJS = $(OBJDIR)/rpc_main.o
$(OBJDIR)/rpc_main.o: rtRemoteConfig.h

$(OBJDIR)/%.o: ../%.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CXX_PRETTY) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o: %.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CXX_PRETTY) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o : ../%.c
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC_PRETTY) -c $(CFLAGS) $< -o $@

debug:
	@echo $(OBJS)

$(RTMSG_LIBNAME): $(RTMSG_OBJS)
	$(CXX_PRETTY) $(RTMSG_OBJS) $(LDFLAGS) -shared -o $(RTMSG_LIBNAME)

$(RTMSGD_TARGET): $(RTMSGD_OBJS) $(RTMSG_LIBNAME)
	$(CXX_PRETTY) $(RTMSGD_OBJS) -L. -l$(RTMSG_TARGET) -L../build/egl -lrtCore -o $(RTMSGD_TARGET)

clean:
	$(RM) -rf $(OBJDIR)
	$(RM) -f $(RTMSG_LIBNAME)
	$(RM) -f $(RTMSGD_TARGET)

.PHONY: all debug clean
