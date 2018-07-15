set(NODEDIR "${EXTDIR}/libnode-v6.9.0/")
set(V8DIR "${EXTDIR}/v8/")

if (WIN32)
    set(V8_INCLUDE_DIRS ${V8DIR}/include
        ${NODEDIR}/deps/uv/include 
        ${NODEDIR}/deps/cares/include
        ${NODEDIR}/deps/openssl/openssl/include
        ${NODEDIR}/deps/icu-small/source/common/unicode
        ${NODEDIR}/deps/icu-small/source/common)
    set(V8_LIBRARY_DIRS ${V8DIR}/out.gn/ia32.release ${NODEDIR}build/Release/lib ${NODEDIR}Release/lib ${NODEDIR}Release)
    set(V8_LIBRARIES
          v8.dll.lib v8_libbase.dll.lib v8_libplatform.dll.lib
          icutools.lib icustubdata.lib icudata.lib icuucx.lib icui18n.lib
          winmm.lib dbghelp.lib shlwapi.lib
          libuv.lib openssl.lib)
elseif (APPLE)
    set(V8_LIBRARY_DIRS ${NODEDIR})
    set (V8_INCLUDE_DIRS ${V8_INCLUDE_DIRS} ${NODEDIR}/deps/icu-small/source/common/)
    set(V8_LIBRARIES node)
else ()
      set (V8_INCLUDE_DIRS ${V8_INCLUDE_DIRS} ${NODEDIR}/deps/icu-small/source/common/)
      set(V8_LIBRARY_DIRS 
          ${NODEDIR}/out/Release/obj.target
          ${NODEDIR}/out/Release/obj.target/deps/v8_inspector/third_party/v8_inspector/platform/v8_inspector
          ${NODEDIR}/out/Release/obj.target/deps/uv
          ${NODEDIR}/out/Release/obj.target/deps/v8/tools/gyp
          ${NODEDIR}/out/Release/obj.target/deps/cares
          ${NODEDIR}/out/Release/obj.target/deps/zlib
          ${NODEDIR}/out/Release/obj.target/deps/http_parser
          ${NODEDIR}out/Release/obj.target/tools/icu
         )
    set(V8_LIBRARIES 
          v8_inspector_stl
          uv
          v8_snapshot
          v8_base
          v8_nosnapshot
          v8_libplatform
          v8_libbase
          cares
          zlib
          http_parser
          icustubdata
          icudata
          icuucx
          icui18n
         )
endif ()

if (NOT SUPPORT_V8)
    unset(V8_INCLUDE_DIRS)
    unset(V8_LIBRARY_DIRS)
    unset(V8_LIBRARIES)
endif (NOT SUPPORT_V8)
