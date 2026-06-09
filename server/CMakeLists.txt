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
)

find_package(PkgConfig REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include(FetchContent)

# Use standalone ASIO bundled with Crow (avoids Boost.ASIO compatibility issues)
set(CROW_USE_BOOST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        Crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG v1.2.1.2
)
FetchContent_MakeAvailable(Crow)

# jwt-cpp (header-only). Prefer vendored copy in libs/jwt-cpp, otherwise fetch.
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
        src/transaction_guard.cpp
        src/dbmanagers/slot_manager.cpp
        src/dbmanagers/user_manager.cpp
        src/dbmanagers/auth_manager.cpp
        src/request_handler.cpp
        src/routes/auth_routes.cpp
        src/routes/queue_routes.cpp
        src/routes/section_routes.cpp
        src/routes/slot_routes.cpp
        src/routes/user_routes.cpp
        src/jwt.cpp
        src/request_handler.cpp
        src/response_builder.cpp
        src/scheduler.cpp
        src/mailer.cpp
)

pkg_check_modules(PQXX REQUIRED libpqxx)

target_link_libraries(Server
        PRIVATE
        ${PQXX_LIBRARIES}
        ${PostgreSQL_LIBRARIES}
        Threads::Threads
        pthread
        OpenSSL::Crypto
        OpenSSL::SSL
        jwt-cpp::jwt-cpp
        Crow::Crow
)

target_include_directories(Server PRIVATE
        ${PQXX_INCLUDE_DIRS}
        ${PostgreSQL_INCLUDE_DIRS}
)

target_compile_options(Server PRIVATE ${PQXX_CFLAGS_OTHER})
target_compile_definitions(Server PRIVATE ASIO_STANDALONE)
