#cmake_minimum_required(VERSION 3.16)

##project(ATE-V5 VERSION 0.1 LANGUAGES CXX)

## Point to your Qt installation
#set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/msvc2022_64")

#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Find Qt modules
#find_package(Qt6 REQUIRED COMPONENTS Widgets Sql SerialPort)

## Project sources
#set(PROJECT_SOURCES
#    main.cpp
#    widget.cpp
#    widget.ui
#)

#qt_add_executable(ATE-V5
#    MANUAL_FINALIZATION
#    ${PROJECT_SOURCES}
#)

## Link Qt libs
##target_link_libraries(ATE-V5
##    PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::SerialPort)
#target_link_libraries(ATE-V5
#    PRIVATE
#        Qt6::Core
#        Qt6::Gui
#        Qt6::Widgets
#        Qt6::Sql
#        Qt6::SerialPort
#        setupapi
#        cfgmgr32
#)

## Windows/macOS bundle settings
#set_target_properties(ATE-V5 PROPERTIES
#    MACOSX_BUNDLE TRUE
#    WIN32_EXECUTABLE TRUE
#)

#include(GNUInstallDirs)
#install(TARGETS ATE-V5
#    BUNDLE DESTINATION .
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)

#qt_finalize_executable(ATE-V5)
cmake_minimum_required(VERSION 3.16)

project(ATE-V6 VERSION 0.1 LANGUAGES CXX)

# Point to your Qt installation (adjust if needed)
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.1/msvc2022_64")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt modules (no SerialPort required)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql SerialPort)

set(PROJECT_SOURCES
    main.cpp
    widget.cpp
    widget.h
    widget.ui
)

qt_add_executable(ATE-V6
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(ATE-V6
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Sql
        Qt6::SerialPort
)

set_target_properties(ATE-V6 PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ATE-V6
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(ATE-V6)
