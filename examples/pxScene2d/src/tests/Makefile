
all: tests

clean:
	rm *.o

#rtZip dependencies
ZLIB=../../external/zlib/
MZ=../../external/zlib/contrib/minizip/

CFLAGS=-ggdb -O -I $(ZLIB) -I $(MZ)


CXXFLAGS=-Wall -Wextra -DRT_PLATFORM_LINUX -I ../ -I $(ZLIB) -I $(MZ)
LDFLAGS=

%.o:	%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o:	%.c
	$(CC) -c $(CXXFLAGS) $< -o $@

SRCS=\
	../utf8.c\
	$(MZ)unzip.c\
	$(MZ)ioapi.c\
	$(ZLIB)libz.a\
	../ioapi_mem.c\
	../rtTest.cpp\
	../rtLog.cpp\
	../rtString.cpp\
	../rtFile.cpp\
	../rtZip.cpp\
	../rtValue.cpp\
	../rtObject.cpp\
	testString.cpp\
	testValue.cpp\
	testObject.cpp\
	testZip.cpp\
	tests.cpp

OBJS=$(patsubst %.cpp, %.o, $(SRCS))
OBJS:=$(patsubst %.c, %.o, $(OBJS))
OBJS: $(SRCS)

tests: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o tests



