# Generated from overview-compositor.pro.

cmake_minimum_required(VERSION 3.14)
project(overview-compositor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/wayland/overview-compositor")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)

add_qt_gui_executable(overview-compositor
    main.cpp
)
target_link_libraries(overview-compositor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Qml
)


# Resources:
set(overview-compositor_resource_files
    "main.qml"
)

qt6_add_resources(overview-compositor "overview-compositor"
    PREFIX
        "/"
    FILES
        ${overview-compositor_resource_files}
)

install(TARGETS overview-compositor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)