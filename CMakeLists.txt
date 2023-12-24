cmake_minimum_required(VERSION 3.16)

project(mbed-HD44780LCD
    DESCRIPTION
        "Library to use the popular HD44780 LCD driver and 16x2 LCD displays with MBed OS"
    LANGUAGES
        CXX
)

add_library(mbed-HD44780LCD INTERFACE)

target_include_directories(mbed-HD44780LCD
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(mbed-HD44780LCD
        INTERFACE
        HD44780LCD.cpp
)
