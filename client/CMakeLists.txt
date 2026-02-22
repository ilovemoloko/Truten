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
        REQUIRED)

add_executable(client src/main.cpp
        include/ApiModel.h src/ApiModel.cpp

        include/GymModelView.h src/GymModelView.cpp
        include/SlotModelView.h src/SlotModelView.cpp
        include/LoginModelView.h src/LoginModelView.cpp
        qml/LoginView.qml
        qml/GymView.qml
        qml/SlotView.qml)
target_link_libraries(client
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

