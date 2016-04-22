LIBRTRPC  = rtRpc
LIBRTCORE = rtCore
SAMPLEAPP = rpcSampleApp

<<<<<<< HEAD
all: lib$(LIBRTRPC).so lib$(LIBRTCORE).so $(SAMPLEAPP)

=======
LIBRTRPC  = rtRpc
LIBRTCORE = rtCore
SAMPLEAPP = rpcSampleApp

all: lib$(LIBRTRPC).so lib$(LIBRTCORE).so $(SAMPLEAPP)

>>>>>>> johnrobinsn/_temp_pxcore2_stage
RTCORE_SRCS=\
  ../utf8.c \
  ../rtString.cpp \
  ../rtLog.cpp \
  ../rtValue.cpp \
<<<<<<< HEAD
  ../rtObject.cpp

RTRPC_SRCS=\
  rtRemoteObjectLocator.cpp \
  rtRemoteObjectResolver.cpp \
=======
  ../rtObject.cpp \
  ../rtError.cpp 


RTRPC_SRCS=\
  rtRpcServer.cpp \
  rtRpcResolver.cpp \
>>>>>>> johnrobinsn/_temp_pxcore2_stage
  rtRemoteObject.cpp \
  rtRemoteFunction.cpp \
  rtRpcMessage.cpp \
  rtRpcClient.cpp \
  rtValueReader.cpp \
  rtValueWriter.cpp \
  rtSocketUtils.cpp \
  rtRpcStream.cpp \
  rtObjectCache.cpp \
  rtRpc.cpp \
  rtRpcConfig.cpp

SAMPLEAPP_SRCS=\
  rpc_main.cpp

SAMPLEAPP_SRCS=\
  rpc_main.cpp

ifeq ($V, 1)
  CXX_PRETTY = $(CXX)
  LD_PRETTY = $(CXX)
  CC_PRETTY = $(CC)
else
  CXX_PRETTY = @echo " [CXX] $<" ; $(CXX)
  LD_PRETTY = @echo "[LINK] $@" ; $(CXX)
  CC_PRETTY = @echo " [CC] $<" ; $(CC)
endif

ifeq ($(DEBUG), 1)
  CXXFLAGS += -g -O0 -fno-inline -DRT_RPC_DEBUG
  CFLAGS   += -g -O0 -fno-inline
else
  CXXFLAGS += -O2
  CFLAGS   += -O2
endif

CFLAGS+=-DRAPIDJSON_HAS_STDSTRING -Wall -Wextra -g -DRT_PLATFORM_LINUX -I../ -I. -fPIC
<<<<<<< HEAD
CXXFLAGS+=-std=c++0x -DRAPIDJSON_HAS_STDSTIRNG $(CFLAGS)
LDFLAGS =-pthread -ldl
=======
CXXFLAGS+=-std=c++0x $(CFLAGS)
LDFLAGS =-pthread -ldl -luuid
>>>>>>> johnrobinsn/_temp_pxcore2_stage
OBJDIR=obj

RTCORE_OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(RTCORE_SRCS)))
RTCORE_OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(RTCORE_OBJS)))
RTCORE_OBJS:=$(patsubst   %.c, %.o        , $(notdir $(RTCORE_OBJS)))
RTCORE_OBJS:=$(patsubst        %, $(OBJDIR)/%, $(RTCORE_OBJS))

RTRPC_OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(RTRPC_SRCS)))
RTRPC_OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(RTRPC_OBJS)))
RTRPC_OBJS:=$(patsubst        %, $(OBJDIR)/%, $(RTRPC_OBJS))

SAMPLEAPP_OBJS = $(OBJDIR)/rpc_main.o
<<<<<<< HEAD
=======

>>>>>>> johnrobinsn/_temp_pxcore2_stage

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

clean:
	rm -rf obj
	rm -f $(SAMPLEAPP)
	rm -f *.so

lib$(LIBRTCORE).so: $(RTCORE_OBJS)
	$(CXX_PRETTY) $(RTCORE_OBJS) $(LDFLAGS) -shared -o $@

lib$(LIBRTRPC).so: $(RTRPC_OBJS)
	$(CXX_PRETTY) $(RTRPC_OBJS) $(LDFLAGS) -shared -o $@

$(SAMPLEAPP): $(SAMPLEAPP_OBJS) lib$(LIBRTRPC).so lib$(LIBRTCORE).so
	$(CXX_PRETTY) $(SAMPLEAPP_OBJS) $(LDFLAGS) -o $@ -L./ -l$(LIBRTCORE) -l$(LIBRTRPC)
