qt_add_executable(keyboard_calculator_test
    keyboard_calculator_test.cpp
    ../calculator.cpp
    ../calculator.h
    ../calculator.ui
    ../resources.qrc
)

target_include_directories(keyboard_calculator_test PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(keyboard_calculator_test PRIVATE
    Qt6::Widgets
    Qt6::Test
)

add_test(
    NAME keyboard_calculator_test
    COMMAND keyboard_calculator_test
)

# 无头运行，不弹出真实窗口
set_tests_properties(keyboard_calculator_test PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)
