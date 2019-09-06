cmake_minimum_required(VERSION 2.8)
find_package(PkgConfig REQUIRED)

pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
find_package(ZLIB REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)

include_directories(include)
include_directories(${LIBXML2_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})

link_directories(${LIBXML2_LIBRARY_DIRS})
link_directories(${CURL_LIBRARY_DIRS})

file(GLOB_RECURSE libdash_source *.cpp)

add_library(dash SHARED ${libdash_source})
target_link_libraries(dash ${CURL_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBXML2_LIBRARIES} -pthread)
install(TARGETS dash DESTINATION lib)

