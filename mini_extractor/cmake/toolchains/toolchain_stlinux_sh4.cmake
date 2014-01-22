# Cross compilation with STLinux SH4 toolchain
# Use it with "cmake -DCMAKE_TOOLCHAIN_FILE=toolchain_stlinux_sh4.cmake"
#
# Documentation : http://www.itk.org/Wiki/CMake_Cross_Compiling#The_toolchain_file

SET(CMAKE_SYSTEM_NAME STLinux-SH4)
SET(CMAKE_SYSTEM_VERSION 2.4)

# Specify the cross compiler
SET(CMAKE_C_COMPILER   /opt/STM/STLinux-${CMAKE_SYSTEM_VERSION}/devkit/sh4/bin/sh4-linux-gcc)
SET(CMAKE_CXX_COMPILER /opt/STM/STLinux-${CMAKE_SYSTEM_VERSION}/devkit/sh4/bin/sh4-linux-g++)

SET(CMAKE_SKIP_COMPATIBILITY_TESTS 1)
SET(CMAKE_C_COMPILER_WORKS 1)
SET(CMAKE_CXX_COMPILER_WORKS 1)
SET(CMAKE_COMPILER_IS_GNUCC_RUN 1)
SET(CMAKE_COMPILER_IS_GNUCXX_RUN 1)

# Generate .o instead of .obj (CMake >= 2.8.4 needed)
SET(CMAKE_C_OUTPUT_EXTENSION .o)
SET(CMAKE_CXX_OUTPUT_EXTENSION .o)

# Location of the target environment 
SET(CMAKE_FIND_ROOT_PATH /opt/STM/STLinux-${CMAKE_SYSTEM_VERSION}/devkit/sh4/target/)

# Search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
