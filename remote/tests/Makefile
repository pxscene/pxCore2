CXX=g++
CXXFLAGS= -L /opt/gtest/lib ../librtCore.so ../librtRemote.so -lgtest -lgtest_main -std=c++11 -lm
INCS= -I /opt/gtest/include
OBJS=rtRpcTest.cpp

testAll:$(OBJS)
	$(CXX) $(INCS) $(OBJS) $(CXXFLAGS) -o testAll
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCS)
clean:
	rm testAll *.o testAll.xml
