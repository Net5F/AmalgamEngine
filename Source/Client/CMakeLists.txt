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
        ${SDL2_GFX_LIBRARIES} ${SDL2PP_LIBRARIES}
        SDL2pp
        AmalgamUI
        Bitsery::bitsery
        readerwriterqueue
        CircularBuffer
        EnTT::EnTT
        QueuedEvents
        Shared
)

# Compile with C++20
target_compile_features(Client PRIVATE cxx_std_20)
set_target_properties(Client PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(Client PUBLIC -Wall -Wextra)
elseif()
    target_compile_options(Client PUBLIC -Wall)
endif(CMAKE_COMPILER_IS_GNUCXX)

# Copy assets to the build directory.
file(COPY ${PROJECT_SOURCE_DIR}/Resources/Client/Assets/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Assets/)

# Copy SpriteData.json to the build directory.
file(COPY ${PROJECT_SOURCE_DIR}/Resources/Shared/SpriteData.json
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# Build all of the subdirectories
add_subdirectory(Config)
add_subdirectory(Launch)
add_subdirectory(Network)
add_subdirectory(Renderer)
add_subdirectory(Simulation)
add_subdirectory(UserInterface)
add_subdirectory(Utility)
