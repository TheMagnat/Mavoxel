
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

#Manually selected shaders
#Note: dependencies of a shader must be added using the ":" symbol to separate them
set(shadersToCompile
    simple_voxel.vert simple_voxel.frag
    basic_color.vert select_color.frag
    sun_color.frag
    only_color.vert only_color.frag
    only_color_uni.vert only_color_uni.frag
    only_texPos.vert only_texPos.frag
    filter.frag
    ray_tracing.frag:raycastLight.frag:octree.frag:lightHelper.frag:materials.frag:octreeHelper.frag
)

# Create the directory for shaders
add_custom_target(CreateShaderDirectory
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:app>/Shaders
    VERBATIM
)

# Create a custom target for compiling and copying shaders
add_custom_target(ShadersTarget
    COMMENT "[SHADER] Starting shaders compilation"
    DEPENDS CreateShaderDirectory
    VERBATIM
)

# Iterate over the shader files and add commands to compile each one
foreach(shaderFiles ${shadersToCompile})

    string(REPLACE ":" ";" shaderFilesList ${shaderFiles})
    list (GET shaderFilesList 0 shaderFile)
    list(TRANSFORM shaderFilesList PREPEND ${CMAKE_SOURCE_DIR}/resources/Shaders/)

    set(shaderInPath  resources/Shaders/${shaderFile})
    set(shaderOutPath Shaders/${shaderFile}.spv)
    set(testShaderOutPath ${CMAKE_SOURCE_DIR}/build/App/Release/Shaders/${shaderFile}.spv)

    # message("here: ${CMAKE_BINARY_DIR} / : ${CMAKE_MSVCIDE_RUN_PATH}")
    add_custom_command(OUTPUT ${testShaderOutPath}
        COMMAND glslangValidator -V ${CMAKE_SOURCE_DIR}/${shaderInPath} -gVS -o $<TARGET_FILE_DIR:app>/${shaderOutPath}
        -DMAX_DEPTH=${svoDepth} -DDEPTH_TO_LEN=${depthToLen} -DRAYTRACING_CHUNK_RANGE=${raytracingChunkRange}
        DEPENDS ${shaderFilesList}
        COMMENT "[SHADER] ${shaderFile} -> ${shaderFile}.spv"
    )

    # Add the custom command as a dependency of the Shaders target
    add_custom_target(${shaderFile}Target DEPENDS ${testShaderOutPath})
    add_dependencies(ShadersTarget ${shaderFile}Target)

endforeach()

#Link the shaders with the app
add_dependencies(app ShadersTarget)

#Libs
add_custom_command(TARGET app POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/resources/dynamicLibs $<TARGET_FILE_DIR:app>
                COMMENT "Copy dynamic libs in binary folder")

## Preprocessor macro
target_compile_definitions(app PRIVATE TIME)
target_compile_definitions(app PRIVATE RAY_CAST)
target_compile_definitions(app PRIVATE SVO_DEPTH=${svoDepth})
target_compile_definitions(app PRIVATE RAYTRACING_CHUNK_RANGE=${raytracingChunkRange})
