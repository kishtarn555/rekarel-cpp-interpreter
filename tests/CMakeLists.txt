cmake_minimum_required(VERSION 3.30)

set(This ReKarelInterpreterTests)

set(Sources
    test_karel.cpp
)

add_executable(${This} ${Sources})

target_link_libraries(${This} PUBLIC
    GTest::gtest_main
    ReKarelInterpreter
)

include(GoogleTest)
gtest_discover_tests(${This})
