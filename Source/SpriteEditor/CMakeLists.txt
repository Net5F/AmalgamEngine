cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring SpriteEditor")

add_executable(SpriteEditor "")

target_include_directories(SpriteEditor
    PRIVATE
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2PP_INCLUDE_DIRS}
)

# Inherit Shared's precompiled header.
target_precompile_headers(SpriteEditor REUSE_FROM Shared)

target_link_libraries(SpriteEditor
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2_GFX_LIBRARIES} ${SDL2PP_LIBRARIES}
        SDL2pp
        Shared
)

# Compile with C++17
target_compile_features(SpriteEditor PRIVATE cxx_std_20)
set_target_properties(SpriteEditor PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
target_compile_options(SpriteEditor PUBLIC -Wall -Wextra)

add_custom_command(
        TARGET SpriteEditor POST_BUILD
        
        # Copy resource files to the build directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/SpriteEditor/Resources/
                ${CMAKE_CURRENT_BINARY_DIR}/Resources/
)

# Build all of the subdirectories
add_subdirectory(Config)
add_subdirectory(Launch)
add_subdirectory(Renderer)
add_subdirectory(UserInterface)
