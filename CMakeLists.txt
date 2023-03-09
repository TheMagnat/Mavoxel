cmake_minimum_required(VERSION 3.1...3.26)

project(
    Mavoxel
    VERSION 0.1.0
    DESCRIPTION "A Voxel engine project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)


if ( MSVC )
    #TODO: Faire plusieurs conf en fonction du système
    set(LIB_PATH "C:/libs/dlls")
    set(LIB_HEADERS_PATH "C:/libs/headers")
endif ( MSVC )


#include(FetchContent)


message("Mavoxel Arch: ${CMAKE_SYSTEM_PROCESSOR}") 

option(MAIN_NOISE "Build noise main" OFF)


add_subdirectory(src output)
