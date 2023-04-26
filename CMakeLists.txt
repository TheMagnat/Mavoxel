cmake_minimum_required(VERSION 3.16...3.26)

project(
    Mavoxel
    VERSION 0.1.0
    DESCRIPTION "A Voxel engine project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)


if ( MSVC )

    message("MSVC Detected : ${MSVC}") 


    #TODO: Faire plusieurs conf en fonction du système
    set(LIB_PATH "C:/libs/dlls")
    set(LIB_HEADERS_PATH "C:/libs/headers")

elseif ( APPLE )

    message("Apple detected") 

    set(LIB_PATH "/usr/local/lib")
    set(LIB_HEADERS_PATH "/usr/local/include")

endif ( MSVC )

## Extern libs and headers
link_directories(${LIB_PATH})
include_directories(${LIB_HEADERS_PATH})


message("Mavoxel Arch: ${CMAKE_SYSTEM_PROCESSOR}") 

option(MAIN_NOISE "Build noise main" OFF)


add_subdirectory(src Mavoxel)
add_subdirectory(app App)
