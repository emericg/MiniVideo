project(mini_thumbnailer)
cmake_minimum_required(VERSION 2.6)


# Find the custom "LibMiniVideo" module
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

# FindLibMiniVideo.cmake
FIND_PACKAGE(LibMiniVideo REQUIRED)

IF(NOT LIBMINIVIDEO_FOUND)
    MESSAGE(ERROR " > LibMiniVideo NOT FOUND !!!")
ENDIF(NOT LIBMINIVIDEO_FOUND)


# FindDoxygen.cmake
FIND_PACKAGE(Doxygen)

IF(NOT DOXYGEN_FOUND)
    MESSAGE(WARNING " > Doxygen not found")
ENDIF(NOT DOXYGEN_FOUND)


# Use "release" and "-O2" for maximum speed.
# Use "debug" if you want better support for GDB and Valgrind
SET(CMAKE_BUILD_TYPE debug)
SET(CMAKE_C_FLAGS "-O0")


# Source files
SET(mini_thumbnailer_SRC src/main.cpp src/main.h)


# Build the thumbnailer software
ADD_EXECUTABLE(mini_thumbnailer ${mini_thumbnailer_SRC})
INCLUDE_DIRECTORIES(${LIBMINIVIDEO_INCLUDE_DIR})

## Link against STATIC library:
## Please set FIND_PACKAGE manually
#FIND_PACKAGE(JPEG) # FindJPEG.cmake
#FIND_PACKAGE(PNG) # FindPNG.cmake
#ADD_LIBRARY(LibMiniVideo STATIC IMPORTED)
#SET_PROPERTY(TARGET LibMiniVideo PROPERTY IMPORTED_LOCATION ../../lib/build/LibMiniVideo.a)
#TARGET_LINK_LIBRARIES(mini_thumbnailer LibMiniVideo ${JPEG_LIBRARY} ${PNG_LIBRARY})


## Link against SHARED library:
LINK_DIRECTORIES(${LibMiniVideo_LIBRARY})
TARGET_LINK_LIBRARIES(mini_thumbnailer "${LIBMINIVIDEO_LIBRARY}" )

# Instal the thumbnailer software into the system (optional step, requires root credentials)
INSTALL(TARGETS mini_thumbnailer DESTINATION /usr/bin)
