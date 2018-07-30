set(NODEDIR "${EXTDIR}/libnode-v6.9.0/")
set(V8DIR "${EXTDIR}/v8/")
set(V8_INCLUDE_DIRS ${V8DIR}/include
        ${NODEDIR}/deps/uv/include 
        ${NODEDIR}/deps/cares/include
        ${NODEDIR}/deps/openssl/openssl/include
        ${NODEDIR}/deps/icu-small/source/common/unicode
        ${NODEDIR}/deps/icu-small/source/common)

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
