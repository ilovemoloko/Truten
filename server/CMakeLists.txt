#hell if I know what's going on here. May God have mercy on our souls
#for the abomination we have created in this very file is despicable.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
cmake_minimum_required(VERSION 3.20)
project(Truten_Server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/routes
        ${CMAKE_CURRENT_SOURCE_DIR}/include/dbmanagers
        /usr/include
)

find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include(FetchContent)

FetchContent_Declare(
        Crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG v1.2.0
)
FetchContent_MakeAvailable(Crow)

# jwt-cpp (header-only). Prefer vendored submodule in libs/jwt-cpp, otherwise FetchContent.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/libs/jwt-cpp/CMakeLists.txt)
    add_subdirectory(libs/jwt-cpp)
else()
    FetchContent_Declare(jwt-cpp
            GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
            GIT_TAG v0.7.0
    )
    FetchContent_MakeAvailable(jwt-cpp)
endif()

add_executable(Server src/main.cpp
        src/database.cpp
        include/dbmanagers/auth_manager.hpp
        include/dbmanagers/user_manager.hpp
        src/slot_manager.cpp
        src/user_manager.cpp
        src/auth_manager.cpp
)

pkg_check_modules(PQXX REQUIRED libpqxx)

target_link_libraries(Server
        PRIVATE
        ${PQXX_LIBRARIES}
        ${PostgreSQL_LIBRARIES}
        Boost::system
        Threads::Threads
        pthread
        OpenSSL::Crypto
        jwt-cpp::jwt-cpp
)

target_include_directories(Server PRIVATE
        ${PQXX_INCLUDE_DIRS}
        ${PostgreSQL_INCLUDE_DIRS}
        ${crow_SOURCE_DIR}/include
)

target_compile_options(Server PRIVATE ${PQXX_CFLAGS_OTHER})
target_compile_definitions(Server PRIVATE ASIO_USE_BOOST=1)