# - Find the "minivideo" includes and library.
#
# This module defines:
#  MINIVIDEO_FOUND, TRUE if we have found libminivideo includes and library.
#  MINIVIDEO_VERSION_STRING, version string.
#  MINIVIDEO_INCLUDE_DIR, where to find "minivideo.h".
#  MINIVIDEO_LIBRARY, the path to the "minivideo" library.

find_path(MINIVIDEO_INCLUDE_DIR
    NAME minivideo.h
    PATHS
    ../minivideo/src
    /usr/include
    /usr/local/include
    /opt/local/include
    /home/your_user/you_can_add_your_custom_path_here
)

find_library(MINIVIDEO_LIBRARY
    NAME minivideo
    PATHS
    ../minivideo/build
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /home/your_user/you_can_add_your_custom_path_here
)

if(MINIVIDEO_INCLUDE_DIR AND EXISTS "${MINIVIDEO_INCLUDE_DIR}/minivideo_settings.h")
  file(STRINGS "${MINIVIDEO_INCLUDE_DIR}/minivideo_settings.h" MINIVIDEO_VERSION_MAJOR_LINE REGEX "^#define[ \t]+minivideo_VERSION_MAJOR[ \t]+[0-9]+$")
  file(STRINGS "${MINIVIDEO_INCLUDE_DIR}/minivideo_settings.h" MINIVIDEO_VERSION_MINOR_LINE REGEX "^#define[ \t]+minivideo_VERSION_MINOR[ \t]+[0-9]+$")
  file(STRINGS "${MINIVIDEO_INCLUDE_DIR}/minivideo_settings.h" MINIVIDEO_VERSION_PATCH_LINE REGEX "^#define[ \t]+minivideo_VERSION_PATCH[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+minivideo_VERSION_MAJOR[ \t]+([0-9]+)$" "\\1" MINIVIDEO_VERSION_MAJOR "${MINIVIDEO_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+minivideo_VERSION_MINOR[ \t]+([0-9]+)$" "\\1" MINIVIDEO_VERSION_MINOR "${MINIVIDEO_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+minivideo_VERSION_PATCH[ \t]+([0-9]+)$" "\\1" MINIVIDEO_VERSION_PATCH "${MINIVIDEO_VERSION_PATCH_LINE}")
  set(MINIVIDEO_VERSION_STRING ${MINIVIDEO_VERSION_MAJOR}.${MINIVIDEO_VERSION_MINOR}.${MINIVIDEO_VERSION_PATCH})
  unset(MINIVIDEO_VERSION_MAJOR_LINE)
  unset(MINIVIDEO_VERSION_MINOR_LINE)
  unset(MINIVIDEO_VERSION_PATCH_LINE)
  unset(MINIVIDEO_VERSION_MAJOR)
  unset(MINIVIDEO_VERSION_MINOR)
  unset(MINIVIDEO_VERSION_PATCH)
endif()

set(MINIVIDEO_FOUND TRUE)

if(NOT MINIVIDEO_INCLUDE_DIR)
    set(MINIVIDEO_FOUND FALSE)
endif()

if(NOT MINIVIDEO_LIBRARY)
    set(MINIVIDEO_FOUND FALSE)
endif()

if(MINIVIDEO_FOUND)
    message("Found MiniVideo: ${MINIVIDEO_LIBRARY} (found version \"${MINIVIDEO_VERSION_STRING})\")")
endif()
