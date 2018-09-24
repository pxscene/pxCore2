if (NOT WIN32)
    find_package(PkgConfig)

    if (PREFER_SYSTEM_LIBRARIES)
    	if (PREFER_PKGCONFIG)
    		pkg_search_module(ZLIB zlib)
    	endif(PREFER_PKGCONFIG)
    	if (NOT ZLIB_FOUND)
        	find_package(ZLIB REQUIRED)
    	endif(NOT ZLIB_FOUND)
    endif(PREFER_SYSTEM_LIBRARIES)
    if (NOT ZLIB_FOUND)
        message(STATUS "Using built-in zlib library")
        set(ZLIB_INCLUDE_DIRS "${EXTDIR}/zlib")
        set(ZLIB_LIBRARY_DIRS "${EXTDIR}/zlib")
        set(ZLIB_LIBRARIES "z")
    endif(NOT ZLIB_FOUND)

    if (PREFER_SYSTEM_LIBRARIES)
    	if (PREFER_PKGCONFIG)
    		pkg_search_module(CURL libcurl)
    	endif(PREFER_PKGCONFIG)
    	if (NOT CURL_FOUND)
        	find_package(CURL REQUIRED)
    	endif(NOT CURL_FOUND)
    endif(PREFER_SYSTEM_LIBRARIES)
    if (NOT CURL_FOUND)
        message(STATUS "Using built-in curl library")
        set(CURL_INCLUDE_DIRS "${EXTDIR}/curl/include")
        set(CURL_LIBRARY_DIRS "${EXTDIR}/curl/lib/.libs")
        set(CURL_LIBRARIES "curl")
    endif(NOT CURL_FOUND)

    if (PREFER_SYSTEM_LIBRARIES)
      	find_package(JPEG)
    endif(PREFER_SYSTEM_LIBRARIES)
    if (NOT JPEG_FOUND)
        message(STATUS "Using built-in jpeg library")
        set(JPEG_INCLUDE_DIRS "${EXTDIR}/jpg")
        set(JPEG_LIBRARY_DIRS "${EXTDIR}/jpg/.libs")
        if (NOT APPLE)
            set(JPEG_LIBRARIES "jpeg")
        else (NOT APPLE)
            set(JPEG_LIBRARIES "${EXTDIR}/jpg/.libs/libjpeg.a")
        endif (NOT APPLE)
    endif(NOT JPEG_FOUND)

    if (NOT APPLE)
        if (PREFER_SYSTEM_LIBRARIES)
            pkg_search_module(TURBO_JPEG libturbojpeg)
        endif(PREFER_SYSTEM_LIBRARIES)
        if (NOT TURBO_JPEG_FOUND)
            message(STATUS "Checking built-in libturbojpeg library")
            find_file(TURBO_JPEG_SO_FILE libturbojpeg.so PATHS "${EXTDIR}/libjpeg-turbo/.libs" NO_DEFAULT_PATH)
            if (TURBO_JPEG_SO_FILE)
                set(TURBO_JPEG_INCLUDE_DIRS "${EXTDIR}/libjpeg-turbo")
                set(TURBO_JPEG_LIBRARY_DIRS "${EXTDIR}/libjpeg-turbo/.libs")
                set(TURBO_JPEG_LIBRARIES "turbojpeg")
            endif (TURBO_JPEG_SO_FILE)
        endif(NOT TURBO_JPEG_FOUND)

        if ((NOT DISABLE_TURBO_JPEG) AND (TURBO_JPEG_FOUND OR TURBO_JPEG_SO_FILE))
            message(STATUS "Have ENABLE_LIBJPEG_TURBO")
            set(TURBO_DEFINITIONS "-DENABLE_LIBJPEG_TURBO")
        endif((NOT DISABLE_TURBO_JPEG) AND (TURBO_JPEG_FOUND OR TURBO_JPEG_SO_FILE))
    endif (NOT APPLE)

    if (PREFER_SYSTEM_LIBRARIES)
    	if (PREFER_PKGCONFIG)
    		pkg_search_module(PNG libpng)
        endif(PREFER_PKGCONFIG)
    	if (NOT PNG_FOUND)
       		find_package(PNG REQUIRED)
    	endif(NOT PNG_FOUND)
    endif (PREFER_SYSTEM_LIBRARIES)
    if (NOT PNG_FOUND)
        message(STATUS "Using built-in libpng library")
        set(PNG_INCLUDE_DIRS "${EXTDIR}/png")
        set(PNG_LIBRARY_DIRS "${EXTDIR}/png/.libs")
        set(PNG_LIBRARIES "png16")
    endif(NOT PNG_FOUND)

    if (PREFER_SYSTEM_LIBRARIES)
        pkg_search_module(FREETYPE REQUIRED freetype2)
    endif (PREFER_SYSTEM_LIBRARIES)
    if (NOT FREETYPE_FOUND)
        message(STATUS "Using built-in freetype2 library")
        set(FREETYPE_INCLUDE_DIRS "${EXTDIR}/ft/include")
        set(FREETYPE_LIBRARY_DIRS "${EXTDIR}/ft/objs/.libs")
        set(FREETYPE_LIBRARIES "freetype")
    endif(NOT FREETYPE_FOUND)

    if (NOT DFB)
      pkg_search_module(GLEW glew)
      pkg_search_module(GL gl)
      pkg_search_module(EGL egl)
      pkg_search_module(GLESV2 glesv2)
      pkg_search_module(X11 x11)
    endif (NOT DFB)
    pkg_search_module(CRYPTO libcrypto)

    if (PREFER_SYSTEM_LIBRARIES)
        pkg_search_module(OPENSSL openssl)
    endif (PREFER_SYSTEM_LIBRARIES)
    if (NOT OPENSSL_FOUND)
        set(OPENSSL_INCLUDE_DIRS "${EXTDIR}/libnode-v6.9.0/deps/openssl/openssl/include")
    endif (NOT OPENSSL_FOUND)

    pkg_search_module(UV libuv)

    pkg_search_module(WAYLAND_EGL wayland-egl)
    pkg_search_module(WAYLAND_CLIENT wayland-client)

    if (PREFER_SYSTEM_LIBRARIES AND SUPPORT_V8)
        pkg_search_module(ICU_I18N icu-i18n)
        pkg_search_module(ICU_UC icu-uc)
    endif (PREFER_SYSTEM_LIBRARIES AND SUPPORT_V8)

else (NOT WIN32)

    set (VCLIBS ${EXTDIR}/vc.build/builds)

    set(ZLIB_INCLUDE_DIRS "${EXTDIR}/zlib-1.2.11")
    set(ZLIB_LIBRARY_DIRS "${VCLIBS}")
    set(ZLIB_LIBRARIES "zlib.lib")

    set(CURL_INCLUDE_DIRS "${EXTDIR}/curl-7.40.0/include")
    set(CURL_LIBRARY_DIRS "${VCLIBS}")
    set(CURL_LIBRARIES "libcurld.lib")

    set(JPEG_INCLUDE_DIRS "${EXTDIR}/jpeg-9a")
    set(JPEG_LIBRARY_DIRS "${VCLIBS}")
    set(JPEG_LIBRARIES "libjpeg.lib")

    set(PNG_INCLUDE_DIRS "${EXTDIR}/libpng-1.6.28")
    set(PNG_LIBRARY_DIRS "${VCLIBS}")
    set(PNG_LIBRARIES "libpng16.lib")

    set(FREETYPE_INCLUDE_DIRS "${EXTDIR}/freetype-2.8.1/include")
    set(FREETYPE_LIBRARY_DIRS "${VCLIBS}")
    set(FREETYPE_LIBRARIES "freetype281MT_D.lib")

    set(GLEW_INCLUDE_DIRS "${EXTDIR}/glew-2.0.0/include")
    set(GLEW_LIBRARY_DIRS "${VCLIBS}")
    set(GLEW_LIBRARIES "glew32s.lib")

endif (NOT WIN32)



set(COMM_DEPS_DEFINITIONS ${COMM_DEPS_DEFINITIONS} ${TURBO_DEFINITIONS})

set(COMM_DEPS_INCLUDE_DIRS ${COMM_DEPS_INCLUDE_DIRS}
        ${ZLIB_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
        ${JPEG_INCLUDE_DIRS}
  ${TURBO_JPEG_INCLUDE_DIRS}
         ${PNG_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
          ${GL_INCLUDE_DIRS}
         ${EGL_INCLUDE_DIRS}
      ${GLESV2_INCLUDE_DIRS}
      ${CRYPTO_INCLUDE_DIRS}
     ${OPENSSL_INCLUDE_DIRS}
         ${X11_INCLUDE_DIRS}
          ${UV_INCLUDE_DIRS}
    ${ICU_I18N_INCLUDE_DIRS}
      ${ICU_UC_INCLUDE_DIRS}
   )

set(COMM_DEPS_LIBRARY_DIRS ${COMM_DEPS_LIBRARY_DIRS}
        ${ZLIB_LIBRARY_DIRS}
        ${CURL_LIBRARY_DIRS}
        ${JPEG_LIBRARY_DIRS}
  ${TURBO_JPEG_LIBRARY_DIRS}
         ${PNG_LIBRARY_DIRS}
    ${FREETYPE_LIBRARY_DIRS}
        ${GLEW_LIBRARY_DIRS}
          ${GL_LIBRARY_DIRS}
         ${EGL_LIBRARY_DIRS}
      ${GLESV2_LIBRARY_DIRS}
      ${CRYPTO_LIBRARY_DIRS}
     ${OPENSSL_LIBRARY_DIRS}
         ${X11_LIBRARY_DIRS}
          ${UV_LIBRARY_DIRS}
    ${ICU_I18N_LIBRARY_DIRS}
      ${ICU_UC_LIBRARY_DIRS}
   )

set(COMM_DEPS_LIBRARIES ${COMM_DEPS_LIBRARIES}
           ${ZLIB_LIBRARIES}
           ${CURL_LIBRARIES}
           ${JPEG_LIBRARIES}
     ${TURBO_JPEG_LIBRARIES}
            ${PNG_LIBRARIES}
       ${FREETYPE_LIBRARIES}
           ${GLEW_LIBRARIES}
             ${GL_LIBRARIES}
            ${EGL_LIBRARIES}
         ${GLESV2_LIBRARIES}
         ${CRYPTO_LIBRARIES}
        ${OPENSSL_LIBRARIES}
            ${X11_LIBRARIES}
             ${UV_LIBRARIES}
       ${ICU_I18N_LIBRARIES}
         ${ICU_UC_LIBRARIES}
   )
