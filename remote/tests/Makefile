#CXXFLAGS= -L /opt/gtest/lib ../librtCore.so ../librtRemote.so -lgtest -lgtest_main -std=c++11 -lm
#INCS= -I /opt/gtest/include
#OBJS=rtRpcTest.cpp
OBJDIR   = obj
PERF_CXXFLAGS = -Wall -DRT_PLATFORM_LINUX -Wextra -std=c++11 -I. -I../ -I../../src

# how are we supposed to know glut/gles? Why is libpxCore.so
PERF_LDFLAGS  = -L. -L.. -L../../build/glut -lrtRemote -lrtCore -pthread

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
  PERF_CXXFLAGS += -g -O0 -fno-inline -DRT_RPC_DEBUG -DRT_DEBUG
else
  PERF_CXXFLAGS += -O2
endif

perftest: perf_server perf_client perf_driver

perf_server: $(OBJDIR)/perf_server.o
	$(CXX_PRETTY) $^ -o $@ $(PERF_LDFLAGS)

perf_client: $(OBJDIR)/perf_client.o
	$(CXX_PRETTY) $^ -o $@ $(PERF_LDFLAGS)

perf_driver: $(OBJDIR)/perf_driver.o
	$(CXX_PRETTY) $^ -o $@ $(PERF_LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	@[ -d $(OBJDIR) ] || mkdir -p $(OBJDIR)
	$(CXX_PRETTY) -c $(PERF_CXXFLAGS) $< -o $@

clean:
	$(RM) -rf obj
	$(RM) perf_driver
	$(RM) perf_server
	$(RM) perf_client
