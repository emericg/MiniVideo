cmake_minimum_required(VERSION 3.20)

project(pugixml VERSION 1.15 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

################################################################################

# Generic sources
set(CORE_SOURCES
    pugixml.cpp
    pugixml.hpp
    pugiconfig.hpp)

################################################################################

add_library(pugixml OBJECT ${CORE_SOURCES})
add_library(pugixml::pugixml ALIAS pugixml)

target_link_libraries(pugixml PRIVATE ${CORE_LIBRARIES})
target_include_directories(pugixml PUBLIC ${CMAKE_CURRENT_LIST_DIR})
