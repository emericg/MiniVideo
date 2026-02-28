cmake_minimum_required(VERSION 3.13)

project(QHexView VERSION 5.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

option(QHEXVIEW_ENABLE_DIALOGS "BuiltIn dialogs" ON)

add_library(${PROJECT_NAME} STATIC)

find_package(Qt6 COMPONENTS Widgets)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt::Widgets
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources(${PROJECT_NAME}
    PRIVATE
        model/buffer/qdevicebuffer.h
        model/buffer/qhexbuffer.h
        model/buffer/qmappedfilebuffer.h
        model/buffer/qmemorybuffer.h
        model/buffer/qmemoryrefbuffer.h
        model/commands/hexviewcommand.h
        model/commands/insertcommand.h
        model/commands/removecommand.h
        model/commands/replacecommand.h
        model/commands/replacecommand.h
        model/qhexcursor.h
        model/qhexdelegate.h
        model/qhexdocument.h
        model/qhexmetadata.h
        model/qhexoptions.h
        model/qhexutils.h
        qhexview.h

    PRIVATE
        model/commands/hexviewcommand.cpp
        model/commands/insertcommand.cpp
        model/commands/removecommand.cpp
        model/commands/replacecommand.cpp
        model/buffer/qdevicebuffer.cpp
        model/buffer/qhexbuffer.cpp
        model/buffer/qmemorybuffer.cpp
        model/buffer/qmemoryrefbuffer.cpp
        model/buffer/qmappedfilebuffer.cpp
        model/qhexdelegate.cpp
        model/qhexutils.cpp
        model/qhexcursor.cpp
        model/qhexmetadata.cpp
        model/qhexdocument.cpp
        qhexview.cpp
)

if(QHEXVIEW_ENABLE_DIALOGS)
    target_sources(${PROJECT_NAME}
        PRIVATE
            dialogs/hexfinddialog.h
            dialogs/hexfinddialog.cpp
    )

    target_compile_definitions(${PROJECT_NAME}
        PUBLIC
            QHEXVIEW_ENABLE_DIALOGS
    )
endif()
