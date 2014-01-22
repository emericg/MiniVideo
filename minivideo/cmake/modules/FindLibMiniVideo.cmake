# - Find the "minivideo" includes and library.
#
# This module defines:
#  LIBMINIVIDEO_INCLUDE_DIR, where to find "minivideo.h".
#  LIBMINIVIDEO_LIBRARY, the path to the "minivideo" library.
#  LIBMINIVIDEO_FOUND, TRUE if we have found libminivideo includes and library.

SET(LIBMINIVIDEO_FOUND TRUE)

FIND_PATH(LIBMINIVIDEO_INCLUDE_DIR
    NAME minivideo.h
    PATHS
    ../minivideo/src
    /usr/include
    /usr/local/include
    /home/user/your_user/you_can_add_your_custom_path_here
)

FIND_LIBRARY(LIBMINIVIDEO_LIBRARY
    NAME minivideo
    PATHS
    ../minivideo/build
    /usr/lib
    /usr/local/lib
    /home/your_user/you_can_add_your_custom_path_here
)

IF(NOT LIBMINIVIDEO_INCLUDE_DIR)
    SET(LIBMINIVIDEO_FOUND FALSE)
ENDIF(NOT LIBMINIVIDEO_INCLUDE_DIR)

IF(NOT LIBMINIVIDEO_LIBRARY)
    SET(LIBMINIVIDEO_FOUND FALSE)
ENDIF(NOT LIBMINIVIDEO_LIBRARY)

#IF(LIBMINIVIDEO_FOUND)
#    MESSAGE("Found MiniVideo library at: ${LIBMINIVIDEO_LIBRARY}")
#ENDIF(LIBMINIVIDEO_FOUND)
