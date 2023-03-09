
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
target_link_libraries(app glfw3)
target_link_libraries(app opengl32)
target_link_libraries(app gdi32)
target_link_libraries(app FastNoise)

target_compile_features(app PRIVATE cxx_std_17)

### Copy ressources

#Shaders
add_custom_command(TARGET app PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/ressources/Shaders $<TARGET_FILE_DIR:app>/Shaders)


add_custom_command(TARGET app PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_SOURCE_DIR}/dynamicLibs $<TARGET_FILE_DIR:app>)