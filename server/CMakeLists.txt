cmake_minimum_required(VERSION 3.20)
project(Truten_Server)

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

find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

add_executable(Server src/main.cpp
        ../database/src/database.cpp
)

pkg_check_modules(PQXX REQUIRED libpqxx)

target_link_libraries(Server
        PRIVATE
        ${PQXX_LIBRARIES}
        ${PostgreSQL_LIBRARIES}
        Boost::system
        Threads::Threads
        pthread
)

target_include_directories(Server PRIVATE
        ${PQXX_INCLUDE_DIRS}
        ${PostgreSQL_INCLUDE_DIRS}
)

target_compile_options(Server PRIVATE ${PQXX_CFLAGS_OTHER})
target_compile_definitions(Server PRIVATE ASIO_USE_BOOST=1)