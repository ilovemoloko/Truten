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

find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_test(
    NAME TrutenConcurrencyTests
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/concurrency_test.py ${CMAKE_CURRENT_SOURCE_DIR}/../server/build/Server
)

qt_add_executable(TrutenClientRefreshTests
    ClientRefreshTest.cpp
    ../client/include/BaseAPIModel.h
    ../client/src/BaseAPIModel.cpp
    ../client/shared/enumToken.h
)

target_include_directories(TrutenClientRefreshTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../client/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../client/shared
)

target_link_libraries(TrutenClientRefreshTests PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Test
)

add_test(NAME TrutenClientRefreshTests COMMAND TrutenClientRefreshTests)
