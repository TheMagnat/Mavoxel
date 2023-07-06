cmake_minimum_required(VERSION 3.16...3.26)

project(
    Mavoxel
    VERSION 0.1.0
    DESCRIPTION "A Voxel engine project"
    LANGUAGES CXX
)

#Set project global property
set(svoDepth 7)
set(depthToLen "float[MAX_DEPTH](64, 32, 16, 8, 4, 2, 1)")
set(raytracingChunkRange 2) #The number of chunk along each axis in each direction (total in each axis is then 1 + raytracingChunkRange * 2)

#C++ property
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)


if ( MSVC )

    message("MSVC Detected : ${MSVC}") 

    #TODO: Faire plusieurs conf en fonction du système
    set(LIB_PATH "C:/Libraries/Bin")
    set(LIB_HEADERS_PATH "C:/Libraries/Include")

    set(ENV{VULKAN_SDK} "C:/VulkanSDK/1.3.250.0/")
    set(VULKAN_HEADERS_PATH "C:/VulkanSDK/1.3.250.0/Include")

    find_package(Vulkan REQUIRED)

elseif ( APPLE )

    message("Apple detected") 

    set(LIB_PATH "/usr/local/lib")
    set(LIB_HEADERS_PATH "/usr/local/include")

endif ( MSVC )

## Extern libs and headers
link_directories(${LIB_PATH})
include_directories(${LIB_HEADERS_PATH} ${VULKAN_HEADERS_PATH})


option(MAIN_NOISE "Build noise main" OFF)


add_subdirectory(vulkan_wrapper VulkanWrapper)
add_subdirectory(src Mavoxel)
add_subdirectory(app App)
