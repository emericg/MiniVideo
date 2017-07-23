# - Find linux kernel version and memfd availability.
#
# This module defines:
#  MEMFD_FOUND, TRUE if we have found a linux kernel version >= 3.17.
#  KERNEL_RELEASE (ex: 4.10.8-1-ARCH)
#  KERNEL_VERSION (ex: 410)
#  KERNEL_VERSION_MAJOR (ex: 4)
#  KERNEL_VERSION_MINOR (ex: 10)

set(MEMFD_FOUND FALSE)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

    execute_process(COMMAND uname -r
                    OUTPUT_VARIABLE KERNEL_RELEASE
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND uname -r
                    COMMAND cut -d. -f1-2 --output-delimiter=
                    OUTPUT_VARIABLE KERNEL_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND uname -r
                    COMMAND cut -c 3-4
                    OUTPUT_VARIABLE KERNEL_VERSION_MINOR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND uname -r
                    COMMAND cut -c 1-1
                    OUTPUT_VARIABLE KERNEL_VERSION_MAJOR
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(${KERNEL_VERSION})
        if(${KERNEL_VERSION} GREATER 316)
            set(MEMFD_FOUND TRUE)
        endif()
    endif()

endif()
