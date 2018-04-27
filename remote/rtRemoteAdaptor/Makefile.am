
bin_PROGRAMS=rtRemoteAdapter rtSampleServer

AM_CXXFLAGS=-DRT_PLATFORM_LINUX -std=c++11 \
            -I${PKG_CONFIG_SYSROOT_DIR}/usr/include/libparodus  \
            -I${PKG_CONFIG_SYSROOT_DIR}/usr/include/wdmp-c \
            -I${PKG_CONFIG_SYSROOT_DIR}/usr/include/wrp-c $(GLIB_CFLAGS) \
            -I${PKG_CONFIG_SYSROOT_DIR}/usr/include \
            -I${PKG_CONFIG_SYSROOT_DIR}/usr/include/cimplog \
            -I${PKG_CONFIG_SYSROOT_DIR}/usr/include/pxcore

AM_LDFLAGS=-L${PKG_CONFIG_SYSROOT_DIR}/usr/lib

rtRemoteAdapter_LDFLAGS=-llibparodus -lrtCore -lrtRemote -lrdkloggers -lcimplog -lwdmp-c -lwrp-c  -lcjson -lpthread
rtRemoteAdapter_SOURCES=rtRemoteAdapter.cpp 

rtSampleServer_LDFLAGS=-lrtCore -lrtRemote -lrdkloggers -lcimplog -lwdmp-c -lwrp-c  -lcjson -lpthread
rtSampleServer_SOURCES=rtSampleServer.cpp 
