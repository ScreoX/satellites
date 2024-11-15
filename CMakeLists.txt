cmake_minimum_required(VERSION 3.26)
project(satellites)

find_package(Qt5 COMPONENTS Widgets Network REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(MyQtApp
        main.cpp
        mainwindow.cpp
        mainwindow.h
        tle_parser.h
        tle_parser.cpp
)

target_link_libraries(MyQtApp Qt5::Widgets Qt5::Network)
