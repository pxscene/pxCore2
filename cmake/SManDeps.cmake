if (NOT WIN32)

    pkg_search_module(SERVICEMANAGER servicemanager)

    # TODO: remove it in future {
    if (NOT SERVICEMANAGER_FOUND)
        # we set it just for backward compatibility, however it's strongly advised to switch to use servicemanager.pc
        set(SERVICEMANAGER_LIBRARIES "servicemanager")
        # SERVICEMANAGER_INCLUDE_DIRS and SERVICEMANAGER_LIBRARY_DIRS can be set manually while invoking cmake (assuming servicemanager.pc pkg-config file is not available on a platform).
    endif(NOT SERVICEMANAGER_FOUND)
    # }

    pkg_search_module(QT5CORE Qt5Core)

    set(SMAN_DEPS_INCLUDE_DIRS ${SMAN_DEPS_INCLUDE_DIRS}
        ${SERVICEMANAGER_INCLUDE_DIRS}
        ${QT5CORE_INCLUDE_DIRS}
       )

    set(SMAN_DEPS_LIBRARY_DIRS ${SMAN_DEPS_LIBRARY_DIRS}
        ${SERVICEMANAGER_LIBRARY_DIRS}
        ${QT5CORE_LIBRARY_DIRS}
       )

    set(SMAN_DEPS_LIBRARIES ${SMAN_DEPS_LIBRARIES}
        ${SERVICEMANAGER_LIBRARIES}
        ${QT5CORE_LIBRARIES}
       )

endif (NOT WIN32)
