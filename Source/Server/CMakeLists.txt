cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Server")

# Sources and includes are added in included subdirectories.
add_executable(Server "")

# Inherit Shared's precompiled header.
target_precompile_headers(Server REUSE_FROM Shared)

# Link all our dependencies.
if(MINGW)
  # We need the specific order: mingw32, SDL2main, SDL2.
    target_link_libraries(Server PRIVATE mingw32)
endif()

target_link_libraries(Server
    PRIVATE
        SDL2main
        SDL2-static
        SDL2_image-static
        SDL2_net-static
        SDL2pp
        flatbuffers
        Shared
        readerwriterqueue
)

# Compile with C++17
target_compile_features(Server PRIVATE cxx_std_17)
set_target_properties(Server PROPERTIES CXX_EXTENSIONS OFF)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(EXE_NAME "Server.exe")
else()
    set(EXE_NAME "Server")
endif()

add_custom_command(
        TARGET Server POST_BUILD
        
        # Make sure the output directory exists
        COMMAND ${CMAKE_COMMAND} -E make_directory 
                ${PROJECT_BINARY_DIR}/Output/Server/

        # Copy C and C++ dlls to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/Cpp/${CMAKE_SYSTEM_NAME}
                ${PROJECT_BINARY_DIR}/Output/Server/

        # Copy SDL2 dlls to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/SDL2/${CMAKE_SYSTEM_NAME}
                ${PROJECT_BINARY_DIR}/Output/Server/

        # Copy pthread dll to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/Pthread/${CMAKE_SYSTEM_NAME}
                ${PROJECT_BINARY_DIR}/Output/Server/

        # Copy the .exe to the output directory.
        TARGET Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}
                ${PROJECT_BINARY_DIR}/Output/Server/
)

# Build all of the subdirectories
add_subdirectory(Launch)
add_subdirectory(Game)
add_subdirectory(Utility)
