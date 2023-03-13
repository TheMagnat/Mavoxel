
link_directories(${LIB_PATH})
include_directories(${LIB_HEADERS_PATH})

file(GLOB_RECURSE CppSources ${CMAKE_SOURCE_DIR}/src/*.cpp)
set(CppHeaders "${CMAKE_SOURCE_DIR}/src")

add_executable(app ${CppSources})


if ( MSVC )

    target_compile_options(app PRIVATE "/MP")

endif( MSVC )

#Add headers
target_include_directories(app
    PRIVATE
    ${CppHeaders}
)

#Add libs
if ( MSVC )
    
    target_link_libraries(app glfw3)
    target_link_libraries(app opengl32)
    target_link_libraries(app gdi32)
    target_link_libraries(app FastNoise)

elseif ( APPLE )
    
    target_link_libraries(app glfw)
    target_link_libraries(app FastNoise)

endif ( MSVC )

target_compile_features(app PRIVATE cxx_std_17)

### Copy ressources

#Shaders
file(GLOB_RECURSE shaders RELATIVE ${CMAKE_SOURCE_DIR}/ressources/Shaders ${CMAKE_SOURCE_DIR}/ressources/Shaders/*s)
add_custom_command(OUTPUT ${shaders})

foreach(shader IN LISTS shaders)

    add_custom_command(OUTPUT ${shaders}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/ressources/Shaders/${shader}
            $<TARGET_FILE_DIR:app>/Shaders/${shader}
        DEPENDS ${CMAKE_SOURCE_DIR}/ressources/Shaders/${shader}
        APPEND
    )

endforeach()

add_custom_target(copyShaders ALL DEPENDS ${shaders})
add_dependencies(app copyShaders)

#Libs
add_custom_command(TARGET app POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/dynamicLibs $<TARGET_FILE_DIR:app>
                COMMENT "Copy dynamicLibs in binary folder")
