cmake_minimum_required(VERSION 2.8)
project(rtRemoteExample)

set(PXCOREDIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(PXCORESRCDIR ${PXCOREDIR}/src)
set(RTREMOTEINC ${PXCOREDIR}/remote)
include_directories(AFTER ${RTREMOTEINC} ${PXCORESRCDIR})
link_directories(${RTREMOTEINC} ${CMAKE_CURRENT_SOURCE_DIR}/../build/glut ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(sample_client client.cpp)
target_link_libraries(sample_client -lrtRemote -lrtCore -luuid)
target_compile_definitions(sample_client PRIVATE RT_PLATFORM_LINUX RAPIDJSON_HAS_STDSTRING)

add_executable(sample_server server.cpp)
target_link_libraries(sample_server -lrtRemote -lrtCore -luuid)
target_compile_definitions(sample_server PRIVATE RT_PLATFORM_LINUX RAPIDJSON_HAS_STDSTRING)
