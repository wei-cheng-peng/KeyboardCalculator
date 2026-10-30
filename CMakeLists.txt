cmake_minimum_required(VERSION 3.16)

project(KeyboardCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 自动处理 Qt 的元对象、.ui 文件与资源文件
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

qt_add_executable(KeyboardCalculator
    main.cpp
    calculator.cpp
    calculator.h
    calculator.ui
    resources.qrc
)

# MSVC 下强制源码按 UTF-8 解析，避免中文乱码
if(MSVC)
    target_compile_options(KeyboardCalculator PRIVATE /utf-8)
endif()

target_link_libraries(KeyboardCalculator PRIVATE Qt6::Widgets)

# 单元测试
enable_testing()
add_subdirectory(tests)
