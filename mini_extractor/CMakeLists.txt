project(mini_extractor)
cmake_minimum_required(VERSION 2.6)


# Find libminivideo module
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})


# FindLibMiniVideo.cmake
FIND_PACKAGE(LibMiniVideo REQUIRED)
IF(LIBMINIVIDEO_FOUND)
    MESSAGE(STATUS "* LibMiniVideo found")
ENDIF(LIBMINIVIDEO_FOUND)


# FindDoxygen.cmake
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    MESSAGE(STATUS "* Doxygen found")
ENDIF(DOXYGEN_FOUND)


# Use "release" and "-O2" for maximum speed.
# Use "debug" if you want better support for GDB and Valgrind
SET(CMAKE_BUILD_TYPE debug)
SET(CMAKE_C_FLAGS "-O0")


# Source files
SET(mini_extractor_SRC src/main.cpp src/main.h)


# Build the thumbnailer software
ADD_EXECUTABLE(mini_extractor ${mini_extractor_SRC})
INCLUDE_DIRECTORIES(${LIBMINIVIDEO_INCLUDE_DIR})


## Link against STATIC library:
## Please set FIND_PACKAGE manually
#FIND_PACKAGE(JPEG) # FindJPEG.cmake
#FIND_PACKAGE(PNG) # FindPNG.cmake
#ADD_LIBRARY(libminivideo STATIC IMPORTED)
#SET_PROPERTY(TARGET libminivideo PROPERTY IMPORTED_LOCATION ../../lib/build/libminivideo.a)
#TARGET_LINK_LIBRARIES(mini_extractor libminivideo ${JPEG_LIBRARY} ${PNG_LIBRARY})


# Link against SHARED library:
LINK_DIRECTORIES(${LIBMINIVIDEO_LIBRARY})
TARGET_LINK_LIBRARIES(mini_extractor ${LIBMINIVIDEO_LIBRARY})


# Instal the thumbnailer software into the system (optional step, requires root credentials)
INSTALL(TARGETS mini_extractor DESTINATION /usr/bin)
