prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${EXEC_INSTALL_PREFIX}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: ${PROJECT_NAME}
Description: MiniVideo is a video framework developed from scratch in C/C++.
URL: https://github.com/emericg/MiniVideo
Version: ${PROJECT_VERSION}
Libs: -L${LIB_INSTALL_DIR} -lminivideo
Cflags: -I${INCLUDE_INSTALL_DIR}
