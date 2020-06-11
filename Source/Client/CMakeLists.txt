cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Client")

# Sources and includes are added in included subdirectories.
add_executable(Client "")

# Inherit Shared's precompiled header.
target_precompile_headers(Client REUSE_FROM Shared)

# Link all our dependencies.
if(MINGW)
  # We need the specific order: mingw32, SDL2main, SDL2.
    target_link_libraries(Client PRIVATE mingw32)
endif()

target_link_libraries(Client
    PRIVATE
        SDL2-static
        SDL2main
        SDL2_image-static
        SDL2_net-static
        SDL2pp
        flatbuffers
        Shared
        readerwriterqueue
        CircularBuffer
)

# Compile with C++17
target_compile_features(Client PRIVATE cxx_std_17)
set_target_properties(Client PROPERTIES CXX_EXTENSIONS OFF)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(EXE_NAME "Client.exe")
else()
    set(EXE_NAME "Client")
endif()

add_custom_command(
        TARGET Client POST_BUILD
        
        # Make sure the output directory exists
        COMMAND ${CMAKE_COMMAND} -E make_directory 
                ${PROJECT_BINARY_DIR}/Output/Client/

        # Copy C and C++ dlls to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/Cpp/${CMAKE_SYSTEM_NAME}
                ${PROJECT_BINARY_DIR}/Output/Client/

        # Copy SDL2 dlls to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/SDL2/${CMAKE_SYSTEM_NAME}
                ${PROJECT_BINARY_DIR}/Output/Client/

        # Copy pthread dll to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/Pthread/${CMAKE_SYSTEM_NAME}
                ${PROJECT_BINARY_DIR}/Output/Client/

        # Copy resource files to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Resources/
                ${PROJECT_BINARY_DIR}/Output/Client/Resources/

        # Copy the .exe to the output directory.
        TARGET Client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}
                ${PROJECT_BINARY_DIR}/Output/Client/
)

# Build all of the subdirectories
add_subdirectory(Launch)
add_subdirectory(Game)
add_subdirectory(Utility)
