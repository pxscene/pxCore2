# Helper function to parse '#define VARIABLE DIGITS' pattern
function(parse_define file define)
    file(STRINGS ${file} value REGEX "#define ${define} ")
    string(REGEX REPLACE "(#define ${define}[ ]+)([0123456789]+).*" "\\2" value ${value})
    set(${define} ${value} PARENT_SCOPE)
endfunction(parse_define)

# Function retrieves the 'v8' engine version string
function(v8_get_version dest_variable file)
        parse_define(${file} V8_MAJOR_VERSION)
        parse_define(${file} V8_MINOR_VERSION)
        parse_define(${file} V8_BUILD_NUMBER)
        parse_define(${file} V8_PATCH_LEVEL)
        parse_define(${file} V8_IS_CANDIDATE_VERSION)
        set(${dest_variable} "${V8_MAJOR_VERSION}.${V8_MINOR_VERSION}.${V8_BUILD_NUMBER}.${V8_PATCH_LEVEL} [is candidate: ${V8_IS_CANDIDATE_VERSION}]" PARENT_SCOPE)
endfunction(v8_get_version)


if (PREFER_SYSTEM_LIBRARIES)
    message(STATUS "Checking for one of the headers 'v8-version.h'")
    find_path(V8_VERSION_DIR "v8-version.h" PATHS ${V8_INCLUDE_DIRS})

    if (V8_VERSION_DIR)
        v8_get_version(V8_VERSION_STRING ${V8_VERSION_DIR}/v8-version.h)

        message(STATUS "Found v8-version.h header at: ${V8_VERSION_DIR} (found version: ${V8_VERSION_STRING})")

        set(V8_FOUND 1)
        add_definitions(-DUSE_SYSTEM_V8)
        set(V8_LIBRARIES
            v8
            v8_libbase
            v8_libplatform
            uv
            uWS)
    endif(V8_VERSION_DIR)
endif (PREFER_SYSTEM_LIBRARIES)

if (NOT V8_FOUND)
set(NODEDIR "${EXTDIR}/libnode-v6.9.0/")
set(V8DIR "${EXTDIR}/v8/")
set(V8_INCLUDE_DIRS ${V8DIR}/include
        ${NODEDIR}/deps/uv/include 
        ${NODEDIR}/deps/cares/include
        ${NODEDIR}/deps/openssl/openssl/include
        ${NODEDIR}/deps/icu-small/source/common/unicode
        ${NODEDIR}/deps/icu-small/source/common)

    message(STATUS "Checking for one of the headers 'v8-version.h'")
    find_path(V8_VERSION_DIR "v8-version.h" PATHS ${V8_INCLUDE_DIRS} NO_DEFAULT_PATH)

    if (V8_VERSION_DIR)
        v8_get_version(V8_VERSION_STRING ${V8_VERSION_DIR}/v8-version.h)

        message(STATUS "Found v8-version.h header at: ${V8_VERSION_DIR} (found version: ${V8_VERSION_STRING})")
    endif (V8_VERSION_DIR)

if (WIN32)
    set(V8_LIBDIR ${V8DIR}/out.gn/ia32.release)
    set(V8_LIBRARY_DIRS ${V8_LIBDIR} ${NODEDIR}build/Release/lib ${NODEDIR}Release/lib ${NODEDIR}Release)
    set(V8_LIBRARIES
          v8.dll.lib v8_libbase.dll.lib v8_libplatform.dll.lib
          icutools.lib icustubdata.lib icudata.lib icuucx.lib icui18n.lib
          winmm.lib dbghelp.lib shlwapi.lib
          libuv.lib openssl.lib)
    set(V8_BUILD_PATH_CHECK ${V8_LIBDIR}/v8.dll.lib)
    set(V8_SOURCES ${V8_LIBDIR}/v8_binfile.cpp)
elseif (APPLE)
    set(V8_LIBDIR ${V8DIR}/out.gn/x64.release/obj)
    set (V8_INCLUDE_DIRS ${V8_INCLUDE_DIRS} ${NODEDIR}/deps/icu-small/source/common/)
    set(V8_LIBRARY_DIRS 
        ${V8_LIBDIR}
		${NODEDIR})
    set(V8_LIBRARIES  
          v8_base
          v8_external_snapshot
          v8_libplatform
          v8_libsampler
          v8_libbase
		node)
    set(V8_BUILD_PATH_CHECK ${V8_LIBDIR}/libv8_base.a)
    set(V8_SOURCES ${V8_LIBDIR}/../v8_binfile.cpp)
else ()
      set(V8_LIBDIR ${V8DIR}/out.gn/x64.release/obj)
      set (V8_INCLUDE_DIRS ${V8_INCLUDE_DIRS} ${NODEDIR}/deps/icu-small/source/common/)
      set(V8_LIBRARY_DIRS 
          ${V8_LIBDIR}
          ${NODEDIR}/out/Release/obj.target
          ${NODEDIR}/out/Release/obj.target/deps/uv
          ${NODEDIR}/out/Release/obj.target/deps/v8/tools/gyp
          ${NODEDIR}/out/Release/obj.target/deps/cares
          ${NODEDIR}/out/Release/obj.target/deps/zlib
          ${NODEDIR}/out/Release/obj.target/deps/http_parser
          ${NODEDIR}out/Release/obj.target/tools/icu
         )
    set(V8_LIBRARIES 
          v8_base
          v8_external_snapshot
          v8_libplatform
          v8_libsampler
          v8_libbase
          uv
          cares
          zlib
          http_parser
          icustubdata
          icudata
          icuucx
          icui18n
         )
    set(V8_BUILD_PATH_CHECK ${V8_LIBDIR}/libv8_base.a)
    set(V8_SOURCES ${V8_LIBDIR}/../v8_binfile.cpp)
endif ()

if  (SUPPORT_V8)
    if(EXISTS ${V8_BUILD_PATH_CHECK})
        message("external/v8 library is built successfully")
    else ()
        message(FATAL_ERROR "external/v8 library is not built, please build it first")
    endif ()
else (SUPPORT_V8)
    unset(V8_INCLUDE_DIRS)
    unset(V8_LIBRARY_DIRS)
    unset(V8_LIBRARIES)
    unset(V8_SOURCES)
endif (SUPPORT_V8)

endif (NOT V8_FOUND)
