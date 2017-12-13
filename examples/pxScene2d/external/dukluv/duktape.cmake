set(DUKTAPEDIR ${CMAKE_CURRENT_LIST_DIR}/lib/duktape)
set(DUKTAPE_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/duktape/extras/module-duktape/)

include_directories(
  ${DUKTAPEDIR}/src
  ${DUKTAPE_MODULE_DIR}
)

add_library(duktape STATIC ${DUKTAPEDIR}/src/duktape.c ${DUKTAPE_MODULE_DIR}/duk_module_duktape.c)

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
  target_link_libraries(duktape
    m dl rt
  )
endif()
