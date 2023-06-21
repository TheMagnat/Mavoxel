
## Prepare sources and headers
file(GLOB_RECURSE CppSources ${CMAKE_SOURCE_DIR}/src/*.cpp)
set(CppHeaders "${CMAKE_SOURCE_DIR}/src")
set(VulkanWrapperHeaders "${CMAKE_SOURCE_DIR}/vulkan_wrapper")

## Create new library target
add_library(Mavoxel ${CppSources})

# Add threading for MSVC compilation
if ( MSVC )
    target_compile_options(Mavoxel PRIVATE "/MP")
endif( MSVC )

## Add headers
target_include_directories(Mavoxel
    PRIVATE
    ${CppHeaders}
)

# Vulkan Wrapper headers
target_include_directories(Mavoxel
    PRIVATE
    ${VulkanWrapperHeaders}
)

## Add libs

#Vulkan Wrapper
target_link_libraries(Mavoxel PRIVATE VulkanWrapper)

if ( MSVC )
    
    target_link_libraries(Mavoxel PRIVATE glfw3)

elseif ( APPLE )
    
    target_link_libraries(Mavoxel PRIVATE glfw)

endif ( MSVC )

## Compilation features
target_compile_features(Mavoxel PRIVATE cxx_std_20)

## Preprocessor macro
target_compile_definitions(Mavoxel PRIVATE NOMINMAX)

target_compile_definitions(Mavoxel PRIVATE TIME)
target_compile_definitions(Mavoxel PRIVATE RAY_CAST)