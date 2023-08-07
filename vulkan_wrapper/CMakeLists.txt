
## Prepare sources and headers
file(GLOB_RECURSE CppSources ${CMAKE_SOURCE_DIR}/vulkan_wrapper/*.cpp)
set(CppHeaders "${CMAKE_SOURCE_DIR}/vulkan_wrapper")

## Create new library target
add_library(VulkanWrapper ${CppSources})

# Add threading for MSVC compilation
if ( MSVC )
    target_compile_options(VulkanWrapper PRIVATE "/MP")
endif( MSVC )

## Add headers
target_include_directories(VulkanWrapper
    PRIVATE
    ${CppHeaders}
)

## Add libs
if ( MSVC )
    
    target_link_libraries(VulkanWrapper PRIVATE glfw3)
    target_link_libraries(VulkanWrapper PRIVATE ${Vulkan_LIBRARIES})

elseif ( APPLE )
    
    target_link_libraries(VulkanWrapper PRIVATE glfw)

endif ( MSVC )

## Compilation features
target_compile_features(VulkanWrapper PRIVATE cxx_std_20)

### Copy resources

### Preprocessor macro
target_compile_definitions(VulkanWrapper PUBLIC GLFW_INCLUDE_VULKAN)

target_compile_definitions(VulkanWrapper PUBLIC GLM_FORCE_RADIANS)
target_compile_definitions(VulkanWrapper PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
