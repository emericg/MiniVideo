cmake_minimum_required(VERSION 3.20)

project(qhexedit VERSION 0.9.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

################################################################################

# Generic dependencies
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets)
set(CORE_LIBRARIES Qt::Core Qt::Gui Qt::Widgets)

# Generic sources
set(CORE_SOURCES
    qhexedit.cpp qhexedit.h
    chunks.cpp chunks.h
    color_manager.cpp color_manager.h
    commands.cpp commands.h)

################################################################################

add_library(qhexedit OBJECT ${CORE_SOURCES})
add_library(qhexedit::qhexedit ALIAS qhexedit)

target_link_libraries(qhexedit PRIVATE ${CORE_LIBRARIES})
target_include_directories(qhexedit PUBLIC ${CMAKE_CURRENT_LIST_DIR})
