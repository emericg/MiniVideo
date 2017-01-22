project(mini_thumbnailer)
cmake_minimum_required(VERSION 2.6)


# Search for the custom "MiniVideo" module
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

# FindMiniVideo.cmake
FIND_PACKAGE(MiniVideo REQUIRED)

IF(MINIVIDEO_FOUND)
    MESSAGE(STATUS "* MiniVideo found")
ELSE(MINIVIDEO_FOUND)
    MESSAGE(ERROR " > MiniVideo not found!")
ENDIF(MINIVIDEO_FOUND)


# Use "release" and "-O2" for maximum speed.
# Use "debug" if you want better support for GDB and Valgrind
MESSAGE(STATUS "* Building in DEBUG mode. You can turn off debug mode by editing 'CMakeLists.txt'.")
SET(CMAKE_BUILD_TYPE debug)
SET(CMAKE_C_FLAGS "-O0")


# Source files
SET(mini_thumbnailer_SRC src/main.cpp src/main.h)


# Build the thumbnailer software
ADD_EXECUTABLE(mini_thumbnailer ${mini_thumbnailer_SRC})
INCLUDE_DIRECTORIES(${MINIVIDEO_INCLUDE_DIR})


# Link against SHARED library:
LINK_DIRECTORIES(${MINIVIDEO_LIBRARY})
TARGET_LINK_LIBRARIES(mini_thumbnailer ${MINIVIDEO_LIBRARY} )

# Instal the thumbnailer software into the system (optional step, requires root credentials)
INSTALL(TARGETS mini_thumbnailer DESTINATION /usr/bin)
