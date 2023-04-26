
## Prepare sources and headers
file(GLOB_RECURSE CppSources ${CMAKE_SOURCE_DIR}/src/*.cpp)
set(CppHeaders "${CMAKE_SOURCE_DIR}/src")

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

## Add libs
if ( MSVC )
    
    target_link_libraries(Mavoxel PRIVATE glfw3)
    target_link_libraries(Mavoxel PRIVATE opengl32)
    target_link_libraries(Mavoxel PRIVATE gdi32)

elseif ( APPLE )
    
    target_link_libraries(Mavoxel PRIVATE glfw)

endif ( MSVC )

## Compilation features
target_compile_features(Mavoxel PRIVATE cxx_std_20)

## Preprocessor macro
target_compile_definitions(Mavoxel PRIVATE TIME)
