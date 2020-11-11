cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Client")

add_executable(Client "")

target_include_directories(Client
    PRIVATE
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2PP_INCLUDE_DIRS}
)

# Inherit Shared's precompiled header.
target_precompile_headers(Client REUSE_FROM Shared)

target_link_libraries(Client
    PRIVATE
        -static-libgcc
        -static-libstdc++
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2PP_LIBRARIES}
        SDL2_net-static
        SDL2pp
        Bitsery::bitsery
        Shared
        readerwriterqueue
        CircularBuffer
)

# Compile with C++17
target_compile_features(Client PRIVATE cxx_std_17)
set_target_properties(Client PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
target_compile_options(Client PUBLIC -Wall -Wextra)

add_custom_command(
        TARGET Client POST_BUILD
        
        # Copy resource files to the build directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Resources/
                ${CMAKE_CURRENT_BINARY_DIR}/Resources/
)

# Build all of the subdirectories
add_subdirectory(Launch)
add_subdirectory(Game)
add_subdirectory(Utility)
