cmake_minimum_required(VERSION 3.16)

message(STATUS "Building Client")

add_executable(Client "")

target_include_directories(Client
    PRIVATE
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_NET_INCLUDE_DIRS}
)

# Inherit Shared's precompiled header.
target_precompile_headers(Client REUSE_FROM Shared)

target_link_libraries(Client
    PRIVATE
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        SDL2pp
        flatbuffers
        Shared
        readerwriterqueue
        CircularBuffer
)

# Compile with C++17
target_compile_features(Client PRIVATE cxx_std_17)
set_target_properties(Client PROPERTIES CXX_EXTENSIONS OFF)

# Set up our exe name and output path.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(EXE_NAME "Client.exe")
else()
    set(EXE_NAME "Client")
endif()
set(OUTPUT_DIR "${PROJECT_BINARY_DIR}/Output/Client/")

add_custom_command(
        TARGET Client POST_BUILD
        
        # Make sure the output directory exists
        COMMAND ${CMAKE_COMMAND} -E make_directory 
                ${OUTPUT_DIR}

        # Copy resource files to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Resources/
                ${OUTPUT_DIR}/Resources/

        # Copy the .exe to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}
                ${OUTPUT_DIR}

        # Detect and copy dependencies to the output directory.
        COMMAND ${CMAKE_COMMAND} -P 
                ${PROJECT_SOURCE_DIR}/../CMake/copy_runtime_deps.cmake
                    ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}
                    ${OUTPUT_DIR}
)
                    
# Set the rpath of our executable and dependencies to $ORIGIN (effectively ./)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_custom_command(
            TARGET Client POST_BUILD
                COMMAND find ${OUTPUT_DIR} -type f -maxdepth 1 -exec patchelf --set-rpath '\$\$ORIGIN' {} \\\;
    )
endif()

# Build all of the subdirectories
add_subdirectory(Launch)
add_subdirectory(Game)
add_subdirectory(Utility)
