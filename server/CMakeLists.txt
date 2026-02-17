cmake_minimum_required(VERSION 3.20)
project(Server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/Crow/include
        /usr/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../database/include
)

find_package(Boost REQUIRED COMPONENTS system)
find_package(PostgreSQL REQUIRED)

add_executable(Server src/main.cpp
)

target_compile_definitions(Server PRIVATE ASIO_USE_BOOST=1)

target_link_libraries(Server PRIVATE Boost::system pthread)