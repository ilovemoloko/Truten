cmake_minimum_required(VERSION 3.20)
project(client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED)

add_executable(client
        src/main.cpp
        include/AuthModel.h src/AuthModel.cpp
        include/GymModelView.h src/GymModelView.cpp
        include/SlotModelView.h src/SlotModelView.cpp
        include/LoginModelView.h src/LoginModelView.cpp
        qml/LoginView.qml
        qml/GymView.qml
        qml/SlotView.qml
        include/BaseModel.h src/BaseModel.cpp
        shared/enumToken.h
        include/GymModel.h src/GymModel.cpp
        include/SlotModel.h src/SlotModel.cpp
        )

target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/shared)

target_link_libraries(client PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
)
