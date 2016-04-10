
all: sample

RT_SRCS=\
  ../utf8.c \
  ../rtString.cpp \
  ../rtLog.cpp \
  ../rtValue.cpp \
  ../rtObject.cpp \
  ../rtError.cpp \
  rpc_main.cpp \
  rtRpcServer.cpp \
  rtRemoteObjectResolver.cpp \
  rtRemoteObject.cpp \
  rtRemoteFunction.cpp \
  rtRpcMessage.cpp \
  rtRpcClient.cpp \
  rtValueReader.cpp \
  rtValueWriter.cpp \
  rtSocketUtils.cpp \
  rtRpcStream.cpp \
  rtObjectCache.cpp

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

CFLAGS+=-DRAPIDJSON_HAS_STDSTRING -Wall -Wextra -g -DRT_PLATFORM_LINUX -I../ -I.
CXXFLAGS+=-std=c++0x $(CFLAGS)
LDFLAGS =-pthread -ldl
OBJDIR=obj

OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(RT_SRCS)))
OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(OBJS)))
OBJS:=$(patsubst   %.c, %.o        , $(notdir $(OBJS)))
OBJS:=$(patsubst        %, $(OBJDIR)/%, $(OBJS))

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
	rm -f rpc_sample
	rm -f core

sample: $(OBJS)
	$(CXX_PRETTY) $(OBJS) $(LDFLAGS) -o rpc_sample

