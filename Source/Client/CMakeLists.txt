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

        # Copy C and C++ stl dlls to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/Stl/${CMAKE_SYSTEM_NAME}
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
