
file(GLOB_RECURSE CppSources ${CMAKE_SOURCE_DIR}/app/*.cpp)
set(CppHeaders "${CMAKE_SOURCE_DIR}/app")
set(MavoxelHeaders "${CMAKE_SOURCE_DIR}/src")
set(VulkanWrapperHeaders "${CMAKE_SOURCE_DIR}/vulkan_wrapper")

add_executable(app ${CppSources})

# Add threading for MSVC compilation
if ( MSVC )
    target_compile_options(app PRIVATE "/MP")
endif( MSVC )

## Add headers
target_include_directories(app
    PRIVATE
    ${CppHeaders}
)

# Mavoxel headers
target_include_directories(app
    PRIVATE
    ${MavoxelHeaders}
)

# Vulkan Wrapper headers
target_include_directories(app
    PRIVATE
    ${VulkanWrapperHeaders}
)

## Add libs

# Vulkan Wrapper Lib from the current projet
target_link_libraries(app PRIVATE VulkanWrapper)

# Mavoxel Lib from the current projet
target_link_libraries(app PRIVATE Mavoxel)

# Other libs
target_link_libraries(app PRIVATE FastNoise)

## Compilation features
target_compile_features(app PRIVATE cxx_std_20)

### Copy resources

##Shaders

#We get all shader files
file(GLOB_RECURSE shaders RELATIVE ${CMAKE_SOURCE_DIR}/resources ${CMAKE_SOURCE_DIR}/resources/*spv)

#We create a custom command to generate all our shaders in the output dir
add_custom_command(OUTPUT ${shaders} COMMAND)

#We specify that the name in the output is symbolic and we will not create a file exactly on this path
set_source_files_properties(${shaders} PROPERTIES SYMBOLIC 1)


foreach(shader IN LISTS shaders)

    #We add the copy for each shader
    add_custom_command(OUTPUT ${shaders}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/resources/${shader}
            $<TARGET_FILE_DIR:app>/${shader}
        DEPENDS ${CMAKE_SOURCE_DIR}/resources/${shader}
        APPEND
    )

endforeach()

#We create a custom target to then make the app depend of it
add_custom_target(copyShaders ALL DEPENDS ${shaders})
add_dependencies(app copyShaders)

#Libs
add_custom_command(TARGET app POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/resources/dynamicLibs $<TARGET_FILE_DIR:app>
                COMMENT "Copy dynamic libs in binary folder")

## Preprocessor macro
target_compile_definitions(app PRIVATE TIME)
