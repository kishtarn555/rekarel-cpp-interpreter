cmake_minimum_required(VERSION 3.30)

set(This ReKarelInterpreter)

set(Sources
    test_hello.cpp
)

add_executable(${This} ${Sources})

target_link_libraries(${This} PUBLIC
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${This})
