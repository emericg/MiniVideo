# - Find linux kernel version and memfd availability.
#
# This module defines:
#  MEMFD_FOUND, TRUE if we have found a linux kernel version >= 3.17.
#  uname -r: 4.10.8-1-ARCH
#  LINUX_KERNEL_VERSION (ex: 4.10)

set(MEMFD_FOUND FALSE)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

    execute_process(COMMAND uname -r OUTPUT_VARIABLE UNAME_RESULT OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCH "[0-9]+.[0-9]+" LINUX_KERNEL_VERSION ${UNAME_RESULT})
    #message(-- " Kernel uname: " ${UNAME_RESULT})
    #message(-- " Kernel version: " ${LINUX_KERNEL_VERSION})

    if (LINUX_KERNEL_VERSION VERSION_GREATER_EQUAL 3.17)
        set(MEMFD_FOUND TRUE)
    endif()

endif()
