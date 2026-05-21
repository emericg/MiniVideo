cmake_minimum_required(VERSION 3.20)

project(mini_thumbnailer VERSION 1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################################################################################

message(STATUS ">>> BUILDING ${PROJECT_NAME} v${PROJECT_VERSION} >>>")
message(STATUS "[CMAKE] CMAKE_VERSION               = ${CMAKE_VERSION}")
message(STATUS "[CMAKE] CMAKE_PREFIX_PATH           = ${CMAKE_PREFIX_PATH}")
message(STATUS "[BUILD] QT_HOST_PATH                = ${QT_HOST_PATH}")
message(STATUS "[BUILD] TOOLCHAIN_FILE              = ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "[BUILD] BUILD_TYPE                  = ${CMAKE_BUILD_TYPE}")
message(STATUS "[SYSTEM] (HOST) SYSTEM NAME         = ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (HOST) SYSTEM VERSION      = ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "[SYSTEM] (HOST) SYSTEM PROCESSOR    = ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM NAME       = ${CMAKE_SYSTEM_NAME}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM VERSION    = ${CMAKE_SYSTEM_VERSION}")
message(STATUS "[SYSTEM] (TARGET) SYSTEM PROCESSOR  = ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "[TOP PROJECT] CMAKE_PROJECT_NAME    = ${CMAKE_PROJECT_NAME}")
message(STATUS "[TOP PROJECT] CMAKE_PROJECT_VERSION = ${CMAKE_PROJECT_VERSION}")
message(STATUS "[TOP PROJECT] CMAKE_SOURCE_DIR      = ${CMAKE_SOURCE_DIR}")
message(STATUS "[PROJECT] PROJECT_NAME              = ${PROJECT_NAME}")
message(STATUS "[PROJECT] PROJECT_VERSION           = ${PROJECT_VERSION}")
message(STATUS "[PROJECT] PROJECT_SOURCE_DIR        = ${PROJECT_SOURCE_DIR}")
message(STATUS "[PROJECT] CMAKE_CURRENT_SOURCE_DIR  = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "--------------------------------------------------------------")

################################################################################

# MiniVideo dependency guard (if the target already exists in the super project)
if(NOT TARGET MiniVideo::MiniVideo)
    # Allow custom install prefix: cmake -B build -DMiniVideo_ROOT=/opt/minivideo
    find_package(MiniVideo REQUIRED)
endif()

################################################################################

add_executable(mini_thumbnailer src/main.cpp)

target_link_libraries(mini_thumbnailer PRIVATE MiniVideo::MiniVideo)

################################################################################

set_target_properties(mini_thumbnailer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

include(GNUInstallDirs)
install(TARGETS mini_thumbnailer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
