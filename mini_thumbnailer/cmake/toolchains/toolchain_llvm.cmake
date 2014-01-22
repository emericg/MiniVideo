# Cross compilation with LLVM and CLANG
# Use it with "cmake -DCMAKE_TOOLCHAIN_FILE=toolchain_llvm.cmake"
#
# Documentation : http://www.itk.org/Wiki/CMake_Cross_Compiling#The_toolchain_file
# Original file : http://mhungerford.blogspot.com/2010/10/cmake-and-clangllvm-fun.html

SET(CMAKE_SYSTEM_NAME LLVM-Linux)
SET(CMAKE_SYSTEM_VERSION 3.4)

# Specify the cross compiler
SET(CMAKE_C_COMPILER "/usr/bin/clang")
SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")
SET(CMAKE_RANLIB "/usr/bin/llvm-ranlib" CACHE INTERNAL STRING)
SET(CMAKE_AR "/usr/bin/llvm-ar" CACHE INTERNAL STRING)
SET(CMAKE_LINKER "/usr/bin/llvm-ld" CACHE INTERNAL STRING)
SET(CMAKE_C_LINKER "/usr/bin/llvm-ld")
SET(CMAKE_CXX_LINKER "/usr/bin/llvm-ld")
SET(CMAKE_C_LINK_EXECUTABLE "/usr/bin/llvm-ld <OBJECTS> -o <TARGET> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>")
SET(CMAKE_CXX_LINK_EXECUTABLE "/usr/bin/llvm-ld <OBJECTS> -o <TARGET> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <LINK_LIBRARIES>")

# Compiler flags
SET(CMAKE_C_FLAGS   "-static -std=gnu89 -D_GNU_SOURCE -O4" CACHE INTERNAL STRING)
SET(CMAKE_CXX_FLAGS "-static -D_GNU_SOURCE -O4 -fno-rtti -fno-threadsafe-statics" CACHE INTERNAL STRING)
SET(CMAKE_C_FLAGS_DEBUG "-g" CACHE INTERNAL STRING)
SET(CMAKE_C_FLAGS_RELEASE "-DNDEBUG" CACHE INTERNAL STRING)
SET(CMAKE_CXX_FLAGS_DEBUG "-g" CACHE INTERNAL STRING)
SET(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG" CACHE INTERNAL STRING)

# Generate .o instead of .obj (CMake >= 2.8.4 needed)
SET(CMAKE_C_OUTPUT_EXTENSION .o)
SET(CMAKE_CXX_OUTPUT_EXTENSION .o)

#MESSAGE("CMAKE_C_LINK_EXECUTABLE = ${CMAKE_C_LINK_EXECUTABLE}
#         FLAGS = ${FLAGS}
#         CMAKE_C_LINK_FLAGS = ${CMAKE_C_LINK_FLAGS}
#         LINK_FLAGS = ${LINK_FLAGS}
#         LINK_LIBRARIES = ${LINK_LIBRARIES}")

# Location of the target environment
SET(CMAKE_FIND_ROOT_PATH /usr/bin)

# Search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
