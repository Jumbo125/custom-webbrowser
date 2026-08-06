cmake_minimum_required(VERSION 3.20)

project(CustomBrowserQt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets WebEngineWidgets WebChannel)

set(APP_VERSION ${PROJECT_VERSION})
set(APP_COMPANY "Custom Browser Qt")
set(APP_PRODUCT_NAME "Custom Browser Qt")
set(APP_DESCRIPTION "Custom Browser Qt")
set(APP_INTERNAL_NAME "CustomBrowser")
set(APP_ORIGINAL_FILENAME "CustomBrowser")

configure_file(${CMAKE_SOURCE_DIR}/version.h.in ${CMAKE_BINARY_DIR}/version.h @ONLY)

qt_add_executable(CustomBrowser
    src/main.cpp
    src/BrowserSettings.h
    src/BrowserSettings.cpp
    src/SettingsLoader.h
    src/SettingsLoader.cpp
    src/CliOverrideParser.h
    src/CliOverrideParser.cpp
    src/BrowserBridge.h
    src/BrowserBridge.cpp
    src/BrowserPage.h
    src/BrowserPage.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_include_directories(CustomBrowser PRIVATE ${CMAKE_BINARY_DIR})

target_link_libraries(CustomBrowser PRIVATE
    Qt6::Widgets
    Qt6::WebEngineWidgets
    Qt6::WebChannel
)

install(TARGETS CustomBrowser RUNTIME DESTINATION .)
install(FILES ini.json DESTINATION .)
install(DIRECTORY Assets WebRoot DESTINATION .)
