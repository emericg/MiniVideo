prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${EXEC_INSTALL_PREFIX}
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: MiniVideo
Description: MiniVideo is a video framework developed from scratch in C/C++.
URL: https://github.com/emericg/MiniVideo
Version: @PROJECT_VERSION@
Libs: -L${LIB_INSTALL_DIR} -lminivideo ${EXTRA_LIBS}
Cflags: -I${INCLUDE_INSTALL_DIR}
