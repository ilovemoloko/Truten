cmake_minimum_required(VERSION 3.20)
project(TrutenTests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Network Test REQUIRED)

qt_add_executable(TrutenTests
    ServerTest.h
    ServerTest.cpp
)

target_link_libraries(TrutenTests PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

# Register with CTest so `ctest` works
include(CTest)
add_test(NAME TrutenServerTests COMMAND TrutenTests)
