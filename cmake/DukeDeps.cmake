if (PREFER_SYSTEM_LIBRARIES)
    pkg_search_module(DUKE duktape)
endif(PREFER_SYSTEM_LIBRARIES)
if (NOT DUKE_FOUND)
    message(STATUS "Using built-in duketape libraries")

    set(DUKEDIR "${EXTDIR}/dukluv")

    if (NOT WIN32)
        set(DUKE_INCLUDE_DIRS ${DUKE_INCLUDE_DIRS}
            ${DUKEDIR}/src
            ${DUKEDIR}/lib/duktape/src
            ${DUKEDIR}/lib/uv/include
           )
        set(DUKE_LIBRARY_DIRS ${DUKE_LIBRARY_DIRS} ${DUKEDIR}/build)
        set(DUKE_LIBRARIES ${DUKE_LIBRARIES}
            duktape.a dschema.a duv.a uv.a
           )
    else (NOT WIN32)
        set(DUKE_INCLUDE_DIRS ${DUKE_INCLUDE_DIRS}
            ${DUKEDIR}/src
            ${DUKEDIR}/lib/duktape/src
            ${DUKEDIR}/lib/uv/include
           )
        set(DUKE_LIBRARY_DIRS ${DUKE_LIBRARY_DIRS} ${DUKEDIR}/build/Release)
        set(DUKE_LIBRARIES ${DUKE_LIBRARIES}
            duktape.lib dschema.lib duv.lib uv.lib
           )
    endif (NOT WIN32)
endif(NOT DUKE_FOUND)

if (SUPPORT_DUKTAPE)
    add_definitions(-DRTSCRIPT_SUPPORT_DUKTAPE)
else (SUPPORT_DUKTAPE)
    unset(DUKE_LIBRARIES)
endif (SUPPORT_DUKTAPE)
