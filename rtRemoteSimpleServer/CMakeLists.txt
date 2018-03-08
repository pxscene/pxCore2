cmake_minimum_required(VERSION 2.8)
project(sample_server)

set(PXCOREDIR ${CMAKE_CURRENT_SOURCE_DIR}/../)
set(PXCORESRCDIR ${PXCOREDIR}/src)
set(RTREMOTEINC ${PXCOREDIR}/remote)
include_directories(AFTER ${RTREMOTEINC} ${PXCORESRCDIR})
link_directories(${RTREMOTEINC} ${CMAKE_CURRENT_SOURCE_DIR}/../build/glut ${CMAKE_CURRENT_SOURCE_DIR}/../build/egl)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(sample_server sample_server.cpp)
target_link_libraries(sample_server -lrtRemote -lrtCore -luuid)
target_compile_definitions(sample_server PRIVATE RT_PLATFORM_LINUX RAPIDJSON_HAS_STDSTRING)

