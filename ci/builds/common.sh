#
# Author: Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
#
# Groups of compilation options used to build
# tested versions of Spark
#

#  Available groups   |     Group Description
# -----------------------------------------------------
#               CMAKE options
# -----------------------------------------------------
# spark_common_opts   - used by all tested versions
# spark_force_node6   - used to force build with node 6
# spark_force_node8   - used to force build with node 8
# spark_glut_opts     - options to build GLUT backend
# spark_wayland_opts  - options to build Wayland backend
# -----------------------------------------------------
#               Compiler options
# -----------------------------------------------------
# cxx_common_opts     - options shared by gcc and clang
# clang_specific_opts - clang specific options
# gcc_specific_opts   - gcc specific options
# gcc_asan_opts       - gcc AddressSanitizer options
# gcc_tsan_opts       - gcc ThreadSanitizer options
 
spark_common_opts=(
  '-DBUILD_RTREMOTE_LIBS=ON'
  '-DBUILD_PXSCENE_RASTERIZER_PATH=ON'
  '-DBUILD_WITH_STATIC_NODE=OFF'
  '-DPREFER_SYSTEM_LIBRARIES=ON'
  '-DBUILD_DEBUG_METRICS=ON'

  # TODO: to be removed
  '-DPXSCENE_COMPILE_WARNINGS_AS_ERRORS=OFF'
  '-DPXCORE_COMPILE_WARNINGS_AS_ERRORS=OFF'
)


spark_glut_opts=(
  '-DBUILD_PX_TESTS=ON'
  '-DPXSCENE_TEST_HTTP_CACHE=OFF'
  '-DBUILD_PXSCENE_STATIC_LIB=ON'
)


spark_wayland_opts=(
  '-DPXCORE_WAYLAND_DISPLAY_READ_EVENTS=OFF'
  '-DPXCORE_MATRIX_HELPERS=OFF'
  '-DPXSCENE_DIRTY_RECTANGLES=ON'

  '-DPXCORE_WAYLAND_EGL=ON'
  '-DBUILD_PXSCENE_WAYLAND_EGL=ON'
  '-DBUILD_WITH_GL=ON'
  '-DBUILD_WITH_WAYLAND=ON'

  '-DBUILD_PXSCENE_SHARED_LIB=OFF'
  '-DBUILD_PXSCENE_APP=ON'
)


spark_force_node6=(
  '-DPKG_CONFIG_DISABLE_NODE=OFF'
  '-DPKG_CONFIG_DISABLE_NODE8=ON'
)


spark_force_node8=(
  '-DPKG_CONFIG_DISABLE_NODE=ON'
  '-DPKG_CONFIG_DISABLE_NODE8=OFF'
)


# Preferably don't use any options starting with:
# -Wno-<option-name>
cxx_common_opts=(
  '-UENABLE_EGL_GENERIC'
  '-DUSE_STD_THREADS'
  '-DMESA_EGL_NO_X11_HEADERS'
  '-DPXSCENE_DISABLE_WST_DECODER'
  '-Wall -Werror -Wextra'
  '-pipe -Werror=format-security -fexceptions -fstack-protector-strong --param=ssp-buffer-size=4 -grecord-gcc-switches'
  '-Woverloaded-virtual'
  '-fPIE'
  '-mmpx'
  '-fno-omit-frame-pointer'
  '-O2'
  '-g3'

  # TODO: to be removed
  '-Wno-deprecated-declarations'
  '-Wno-unused-parameter'
)


clang_specific_opts=(
  # TODO: to be removed
  '-Wno-unused-private-field'
  '-Wno-ignored-attributes'
)


gcc_specific_opts=(
  '-Wp,-D_FORTIFY_SOURCE=2'

  # TODO: would be nice to remove below
  # however, Spark is currently written in a way
  # we have to keep it for a while
  # e.g.: rtObjectMacros.h:190:141: warning: nonnull argument ‘r’ compared to NULL
  '-Wno-nonnull-compare'
  '-fno-delete-null-pointer-checks'

  # TODO: tobe removed (currently those below
  # still causes errors on gcc >8.1
  '-Wno-cast-function-type'
  '-Wno-class-memaccess'
)


gcc_asan_opts=(
  '-fsanitize=address'
  '-fsanitize-address-use-after-scope'
  '-fsanitize-recover=address'
  '-fstack-protector-all'
)


gcc_tsan_opts=(
  '-fsanitize=thread'
)



# Common functions

# Compiles rtRemoteConfigGen
compile_rt_remote() {
  cwd=$PWD
  mkdir -p rtRemote
  (cd rtRemote; cmake ../../remote && make "$@" rtRemoteConfigGen)
  echo -DRTREMOTE_GENERATOR_EXPORT=$cwd/rtRemote/rtRemoteConfigGen_export.cmake
}
