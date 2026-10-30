cmake_minimum_required(VERSION 3.16)

project(KeyboardCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理 Qt 的元对象、.ui 文件
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(KeyboardCalculator
    main.cpp
    calculator.cpp
    calculator.h
    calculator.ui
)

# MSVC 下强制源码按 UTF-8 解析，避免中文乱码
if(MSVC)
    target_compile_options(KeyboardCalculator PRIVATE /utf-8)
endif()

target_link_libraries(KeyboardCalculator PRIVATE Qt6::Widgets)
