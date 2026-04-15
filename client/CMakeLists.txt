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
        Quick
        Qml
        REQUIRED)

qt_policy(SET QTP0004 NEW)


add_executable(appclient
        src/main.cpp
        include/AuthModel.h src/AuthModel.cpp
        include/GymModelView.h src/GymModelView.cpp

        include/AuthModelView.h src/AuthModelView.cpp
        include/BaseAPIModel.h src/BaseAPIModel.cpp
        shared/enumToken.h
        include/GymModel.h src/GymModel.cpp
        include/SlotModel.h src/SlotModel.cpp
        )

qt_add_qml_module(appclient
    URI client
    VERSION 1.0
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        qml/AuthView.qml
        qml/GymView.qml

        SOURCES include/SlotsList.h
)

target_include_directories(appclient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/shared)

target_link_libraries(appclient PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Qml
        Qt6::Quick
)
