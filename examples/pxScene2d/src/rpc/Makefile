
all: sample

RT_SRCS=\
  ../utf8.c \
  ../rtString.cpp \
  ../rtLog.cpp \
  ../rtValue.cpp \
  ../rtObject.cpp \
  rpc_main.cpp \
  rtRemoteObjectLocator.cpp \
  rtRemoteObject.cpp

CXXFLAGS=-O0 -DRAPIDJSON_HAS_STDSTRING -Wall -Wextra -std=c++0x -g -DRT_PLATFORM_LINUX -I../ -I.
LDFLAGS =-pthread -ldl
OBJDIR=obj

OBJS =$(patsubst ../%.cpp, %.o        , $(notdir $(RT_SRCS)))
OBJS:=$(patsubst    %.cpp, %.o        , $(notdir $(OBJS)))
OBJS:=$(patsubst   %.c, %.o        , $(notdir $(OBJS)))
OBJS:=$(patsubst        %, $(OBJDIR)/%, $(OBJS))

$(OBJDIR)/%.o: ../%.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o: %.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJDIR)/%.o : ../%.c
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CC) -c $(CXXFLAGS) $< -o $@

debug:
	@echo $(OBJS)

sample: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o rpc_sample

