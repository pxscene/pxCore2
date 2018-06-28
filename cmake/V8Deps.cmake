set(NODEDIR "${EXTDIR}/libnode-v6.9.0/")
set(V8_INCLUDE_DIRS ${NODEDIR}/src ${NODEDIR}/deps/uv/include ${NODEDIR}/deps/v8/include ${NODEDIR}/deps/cares/include)

if (WIN32)
    set(V8_INCLUDE_DIRS ${V8_INCLUDE_DIRS}
          ${NODEDIR}/deps/openssl/openssl/include ${NODEDIR}/deps/http_parser
          ${NODEDIR}/deps/v8_inspector/third_party/v8_inspector/
          ${NODEDIR}/deps/icu-small/source/common/unicode
          ${NODEDIR}/Release/obj/global_intermediate/blink
          ${NODEDIR}/Release/obj/global_intermediate
          ${NODEDIR}/Release/obj/gen/blink
          ${NODEDIR}/deps/icu-small/source/common
          ${NODEDIR}/tools/msvs/genfiles
         )
    set(V8_LIBRARY_DIRS ${NODEDIR}build/Release/lib ${NODEDIR}Release/lib ${NODEDIR}Release)
    set(V8_LIBRARIES
          v8_libplatform.lib v8_libbase.lib v8_nosnapshot.lib v8_snapshot.lib v8_base_0.lib
          v8_base_1.lib v8_base_2.lib v8_base_3.lib
          gtest.lib cares.lib http_parser.lib
          icutools.lib icustubdata.lib icudata.lib icuucx.lib icui18n.lib
          libuv.lib openssl.lib v8_inspector_stl.lib cctest.lib
         )
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
