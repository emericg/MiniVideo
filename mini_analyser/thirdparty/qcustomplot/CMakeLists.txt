cmake_minimum_required(VERSION 3.20)

project(qcustomplot VERSION 2.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

################################################################################

# Generic dependencies
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui)
set(CORE_LIBRARIES Qt::Core Qt::Gui)

# Generic sources
set(CORE_SOURCES
    qcustomplot.cpp
    qcustomplot.h)

################################################################################

add_library(qcustomplot OBJECT ${CORE_SOURCES})
add_library(qcustomplot::qcustomplot ALIAS qcustomplot)

target_link_libraries(qcustomplot PRIVATE ${CORE_LIBRARIES})
target_include_directories(qcustomplot PUBLIC ${CMAKE_CURRENT_LIST_DIR})
