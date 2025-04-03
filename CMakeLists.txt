cmake_minimum_required(VERSION 3.16)
project(PIDTuner VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Widgets SerialPort Charts REQUIRED)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets SerialPort Charts REQUIRED)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/tchartview.cpp
    src/tchartview.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::SerialPort
    Qt::Charts
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    HAVE_SERIALPORT
    HAVE_CHARTS
)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()