cmake_minimum_required(VERSION 3.30)

set(This ReKarelInterpreter)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(Headers
    json.h
    karel.h
    logging.h
    macros.h
    util.h
    world.h
    xml.h
)

set(Sources
    json.cpp
    karel.cpp
    logging.cpp
    util.cpp
    world.cpp
    xml.cpp
)

add_library(${This} STATIC ${Sources} ${Headers})

add_subdirectory(tests)
