# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2018 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
cmake_minimum_required (VERSION 2.8)
project (ABRManager)

set(LIB_SOURCES ABRManager.cpp)

add_library(abr SHARED ${LIB_SOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11 -Wno-multichar")

if(CMAKE_SYSTEMD_JOURNAL)
    message("CMAKE_SYSTEMD_JOURNAL set")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_SYSTEMD_JOURNAL_PRINT=1 -DSD_JOURNAL_SUPPRESS_LOCATION=1")
    target_link_libraries (abr "-lsystemd")
endif()

if(CMAKE_SOC_PLATFORM_INTEL)
	message("CMAKE_SOC_PLATFORM_INTEL set")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_SYSLOG_HELPER_PRINT=1")
	target_link_libraries(abr "-lsysloghelper")
endif()

set_target_properties(abr PROPERTIES PUBLIC_HEADER "ABRManager.h")
install(TARGETS abr DESTINATION lib PUBLIC_HEADER DESTINATION include)
