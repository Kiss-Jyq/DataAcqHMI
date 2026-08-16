cmake_minimum_required(VERSION 3.16)

project(DataAcqHMI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets SerialPort Charts)
qt_standard_project_setup()

add_executable(DataAcqHMI
    src/main.cpp
    src/core/datapoint.h
    src/core/protocolparser.h
    src/core/protocolparser.cpp
    src/core/serialworker.h
    src/core/serialworker.cpp
    src/ui/databuffer.h
    src/ui/databuffer.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.cpp
)

target_link_libraries(DataAcqHMI
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::SerialPort
        Qt6::Charts
)

if(WIN32)
    set_target_properties(DataAcqHMI PROPERTIES WIN32_EXECUTABLE ON)
endif()
