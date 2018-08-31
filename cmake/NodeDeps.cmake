if (PREFER_SYSTEM_LIBRARIES)

    if ((NOT PKG_CONFIG_DISABLE_NODE8) AND (NOT NODE_FOUND))
        pkg_search_module(NODE node8)
    endif((NOT PKG_CONFIG_DISABLE_NODE8) AND (NOT NODE_FOUND))

    if ((NOT PKG_CONFIG_DISABLE_NODE) AND (NOT NODE_FOUND))
        pkg_search_module(NODE node)
    endif((NOT PKG_CONFIG_DISABLE_NODE) AND (NOT NODE_FOUND))

    if (NODE_FOUND)
        message(STATUS "Using external nodejs library (found version \"${NODE_VERSION}\")")
    endif(NODE_FOUND)

endif(PREFER_SYSTEM_LIBRARIES)
if (NOT NODE_FOUND)
    message(STATUS "Using built-in nodejs library")
    if (USE_NODE_0_12_7)
          set(NODEDIR "${EXTDIR}/libnode/")
    else (USE_NODE_0_12_7)
      if (NOT WIN32)
          set(NODEDIR "${EXTDIR}/node/")
      else (NOT WIN32)
          set(NODEDIR "${EXTDIR}/libnode-v6.9.0/")
      endif (NOT WIN32)
    endif (USE_NODE_0_12_7)

    set(NODE_INCLUDE_DIRS ${NODEDIR}/src ${NODEDIR}/deps/uv/include ${NODEDIR}/deps/v8/include ${NODEDIR}/deps/cares/include)
    if (USE_NODE_0_12_7)
          set(NODE_INCLUDE_DIRS ${NODE_INCLUDE_DIRS} ${NODEDIR}/deps/debugger-agent/include)
          set(NODE_INCLUDE_DIRS ${NODE_INCLUDE_DIRS} ${NODEDIR}/deps/v8)
          set(NODE_LIBRARY_DIRS ${NODE_LIBRARY_DIRS} ${NODEDIR}/out/Release)
    endif (USE_NODE_0_12_7)
  

  if (NOT BUILD_WITH_STATIC_NODE)
      if (NOT WIN32)
          set(NODE_LIBRARY_DIRS ${NODE_LIBRARY_DIRS} ${NODEDIR})
          set(NODE_LIBRARIES ${NODE_LIBRARIES} node)
      else (NOT WIN32)
          set(NODE_INCLUDE_DIRS ${NODE_INCLUDE_DIRS}
              ${NODEDIR}/deps/openssl/openssl/include ${NODEDIR}/deps/http_parser
              ${NODEDIR}/deps/v8_inspector/third_party/v8_inspector/
              ${NODEDIR}/deps/icu-small/source/common/unicode
              ${NODEDIR}/Release/obj/global_intermediate/blink
              ${NODEDIR}/Release/obj/global_intermediate
              ${NODEDIR}/Release/obj/gen/blink
              ${NODEDIR}/deps/icu-small/source/common
              ${NODEDIR}/tools/msvs/genfiles
             )
          set(NODE_LIBRARY_DIRS ${NODE_LIBRARY_DIRS} ${NODEDIR}build/Release/lib ${NODEDIR}Release/lib ${NODEDIR}Release)
          set(NODE_LIBRARIES ${NODE_LIBRARIES}
              v8_libplatform.lib v8_libbase.lib v8_nosnapshot.lib v8_snapshot.lib v8_base_0.lib
              v8_base_1.lib v8_base_2.lib v8_base_3.lib
              gtest.lib cares.lib http_parser.lib
              icutools.lib icustubdata.lib icudata.lib icuucx.lib icui18n.lib
              libuv.lib openssl.lib v8_inspector_stl.lib
              node.lib cctest.lib
             )
      endif (NOT WIN32)
  else (NOT BUILD_WITH_STATIC_NODE)
      set(NODE_LIBRARY_DIRS ${NODE_LIBRARY_DIRS}
          ${NODEDIR}/out/Release/obj.target
          ${NODEDIR}/out/Release/obj.target/deps/v8_inspector/third_party/v8_inspector/platform/v8_inspector
          ${NODEDIR}/out/Release/obj.target/deps/uv
          ${NODEDIR}/out/Release/obj.target/deps/v8/tools/gyp
          ${NODEDIR}/out/Release/obj.target/deps/cares
          ${NODEDIR}/out/Release/obj.target/deps/zlib
          ${NODEDIR}/out/Release/obj.target/deps/http_parser
          ${NODEDIR}out/Release/obj.target/tools/icu
         )

      set(NODE_LIBRARIES ${NODE_LIBRARIES}
          node
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
          icui18n
          icuucx
          icudata
         )
  endif(NOT BUILD_WITH_STATIC_NODE)
endif(NOT NODE_FOUND)

if (SUPPORT_NODE)
    add_definitions(-DRTSCRIPT_SUPPORT_NODE)
else (SUPPORT_NODE)
    unset(NODE_LIBRARIES)
    unset(NODE_INCLUDE_DIRS)
    unset(NODE_LIBRARY_DIRS)
endif (SUPPORT_NODE)
