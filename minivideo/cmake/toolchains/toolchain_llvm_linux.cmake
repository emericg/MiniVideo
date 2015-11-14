# Cross compilation with LLVM and CLANG
# Use it with "cmake -DCMAKE_TOOLCHAIN_FILE=toolchain_llvm_linux.cmake"
#
# Documentation : http://www.itk.org/Wiki/CMake_Cross_Compiling#The_toolchain_file

include(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

set(LLVM_HOST)
set(LLVM_TAG LLVM)

# use LLVM from MacPorts
set(LLVM_PATH /usr)

#
# Basic settings
#

set(CMAKE_FIND_ROOT_PATH
    ${LLVM_PATH}/
    ${LLVM_PATH}/include
    ${LLVM_PATH}/usr/include
    )

set(CMAKE_C_COMPILER   ${LLVM_PATH}/bin/clang)
set(CMAKE_CXX_COMPILER ${LLVM_PATH}/bin/clang++)
set(CMAKE_ASM_COMPILER ${LLVM_PATH}/bin/llvm-as)


set(TOOLCHAIN_LLVM_DEBUG 1)

if(TOOLCHAIN_LLVM_DEBUG)
    message(STATUS "${CMAKE_C_COMPILER}")
endif()

set(CMAKE_C_FLAGS
    CACHE STRING "LLVM - GCC/C flags" FORCE
)

set(CMAKE_CXX_FLAGS
    ""
    CACHE STRING "LLVM - GCC/C++ flags" FORCE
)

set(CMAKE_SHARED_LINKER_FLAGS
    ""
    CACHE STRING "LLVM - GCC/C++ flags" FORCE
)

# Search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
