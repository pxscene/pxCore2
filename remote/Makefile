all: rpcSampleApp rpcSampleApp_s

BUILD_CXX ?= $(CXX)

RTCORE_SRCS=\
  ../utf8.c \
  ../rtString.cpp \
  ../rtLog.cpp \
  ../rtValue.cpp \
  ../rtObject.cpp \
  ../rtError.cpp


RTRPC_SRCS=\
  rtRemoteServer.cpp \
  rtRemoteObject.cpp \
  rtRemoteFunction.cpp \
  rtRemoteMessage.cpp \
  rtRemoteClient.cpp \
  rtRemoteValueReader.cpp \
  rtRemoteValueWriter.cpp \
  rtRemoteSocketUtils.cpp \
  rtRemoteStream.cpp \
  rtRemoteObjectCache.cpp \
  rtRemote.cpp \
  rtRemoteConfig.cpp \
  rtRemoteFactory.cpp \
  rtRemoteFileResolver.cpp \
  rtRemoteMulticastResolver.cpp \
  rtRemoteNsResolver.cpp \
  rtRemoteNameService.cpp \
  rtRemoteConfigBuilder.cpp \
  rtRemoteAsyncHandle.cpp \
  rtRemoteUtils.cpp \
  rtRemoteEnvironment.cpp \
  rtRemoteStreamSelector.cpp \
  rtGuid.cpp \
  rtRemoteMapperMap.cpp \
  rtRemoteMapperFile.cpp \
  rtRemoteMapperSql.cpp \
  rtRemoteEndpointHandleStreamServer.cpp

SAMPLEAPP_SRCS=\
  rpc_main.cpp

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

ifeq ($(WITH_KERNEL_GUID), 1)
  CXXFLAGS += -DRT_REMOTE_KERNEL_GUID
endif

CFLAGS+=-DRAPIDJSON_HAS_STDSTRING -Werror -Wall -Wextra -DRT_PLATFORM_LINUX -I../src -I. -fPIC -Wno-deprecated-declarations
CFLAGS+=-DRT_REMOTE_LOOPBACK_ONLY
CXXFLAGS+=-std=c++0x $(CFLAGS)
LDFLAGS =-pthread -ldl -luuid -Wl,-rpath=../../,--enable-new-dtags
OBJDIR=obj

RTCORE_OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(RTCORE_SRCS)))
RTCORE_OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(RTCORE_OBJS)))
RTCORE_OBJS:=$(patsubst   %.c, %.o        , $(notdir $(RTCORE_OBJS)))
RTCORE_OBJS:=$(patsubst        %, $(OBJDIR)/%, $(RTCORE_OBJS))

RTRPC_OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(RTRPC_SRCS)))
RTRPC_OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(RTRPC_OBJS)))
RTRPC_OBJS:=$(patsubst        %, $(OBJDIR)/%, $(RTRPC_OBJS))

SAMPLEAPP_OBJS = $(OBJDIR)/rpc_main.o
$(OBJDIR)/rpc_main.o: rtRemoteConfig.h

RT_REMOTE_CONFIG_GEN=$(OBJDIR)/rtRemoteConfigGen
$(RT_REMOTE_CONFIG_GEN): rtRemoteConfigGen.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(BUILD_CXX_PRETTY) $(CXXFLAGS) $< -o $@

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

$(RTRPC_OBJS): rtRemoteConfig.h rtremote.conf.gen rtRemoteConfigBuilder.cpp

rtremote.conf.gen: rtremote.conf.ac $(RT_REMOTE_CONFIG_GEN)
	$(RT_REMOTE_CONFIG_GEN) -i rtremote.conf.ac -c -o rtremote.conf
	touch rtremote.conf.gen

#rtremote.conf: rtremote.conf.ac $(RT_REMOTE_CONFIG_GEN)
#	$(RT_REMOTE_CONFIG_GEN) -i rtremote.conf.ac -c -o rtremote.conf

rtRemoteConfig.h: $(RT_REMOTE_CONFIG_GEN) rtremote.conf.ac
	$(RT_REMOTE_CONFIG_GEN) -i rtremote.conf.ac -h -o rtRemoteConfig.h

rtRemoteConfigBuilder.cpp: $(RT_REMOTE_CONFIG_GEN) rtremote.conf.ac
	$(RT_REMOTE_CONFIG_GEN) -i rtremote.conf.ac -s -o rtRemoteConfigBuilder.cpp

clean:
	rm -rf obj
	rm -f *.a *.so rpcSampleApp*
	rm -f rtremote.conf.gen
	rm -f rtremote.conf
	rm -f rtRemoteConfig.h

perftest:
	$(MAKE) -C tests perftest

librtRemote.so: $(RTRPC_OBJS)
	$(CXX_PRETTY) $(RTRPC_OBJS) $(LDFLAGS) -shared -o $@

librtRemote_s.a: $(RTRPC_OBJS)
	$(AR) rcs -o $@ $(RTRPC_OBJS)

rpcSampleApp: $(SAMPLEAPP_OBJS) librtRemote.so
	$(CXX_PRETTY) $(SAMPLEAPP_OBJS) $(LDFLAGS) -o $@ -L./ -L../build/glut -L../build/egl -lrtCore -lrtRemote -luuid

rpcSampleApp_s: $(SAMPLEAPP_OBJS) librtRemote_s.a
	$(CXX_PRETTY) $(SAMPLEAPP_OBJS) $(LDFLAGS) -o $@ -L./ -L../build/glut -L../build/egl -lrtCore_s -lrtRemote_s -luuid

rpcSampleSimple: librtRemote.so
	$(CXX_PRETTY) rtSampleClient.cpp -DRT_PLATFORM_LINUX -I. -DRAPIDJSON_HAS_STDSTRING\
    -o rtSampleClient -I../src -L. -L../build/egl -lrtCore -lrtRemote -l uuid -std=c++11
	$(CXX_PRETTY) rtSampleServer.cpp -DRT_PLATFORM_LINUX -I. -DRAPIDJSON_HAS_STDSTRING\
    -o rtSampleServer -I../src -L. -L../build/egl -lrtCore -lrtRemote -l uuid -std=c++11

.PHONY: all debug clean
