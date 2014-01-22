# Cross compilation with MinGW32 i486 toolchain
# Use it with "cmake -DCMAKE_TOOLCHAIN_FILE=toolchain_mingw32.cmake"
#
# Documentation : http://www.itk.org/Wiki/CMake_Cross_Compiling#The_toolchain_file

SET(CMAKE_SYSTEM_NAME MinGW32-i486)
SET(CMAKE_SYSTEM_VERSION 1)

# Specify the cross compiler
SET(CMAKE_C_COMPILER   i486-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i486-mingw32-g++)
SET(CMAKE_RC_COMPILER  i486-mingw32-windres)

# Location of the target environment
SET(CMAKE_FIND_ROOT_PATH /usr/i486-mingw32)

# Search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
