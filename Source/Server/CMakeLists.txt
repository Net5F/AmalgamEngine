add_executable(Server "")

target_include_directories(Server
    PRIVATE
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_NET_INCLUDE_DIRS}
)

target_link_libraries(Server
    PRIVATE
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        SDL2pp
        flatbuffers
        Shared
        readerwriterqueue
)

# Compile with C++17
target_compile_features(Server PRIVATE cxx_std_17)
set_target_properties(Server PROPERTIES CXX_EXTENSIONS OFF)

add_custom_command(
        TARGET Server POST_BUILD
        
        # Make sure the output directory exists
        COMMAND ${CMAKE_COMMAND} -E make_directory 
                ${PROJECT_BINARY_DIR}/Output/Server/

        # Copy C and C++ stl dlls to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/Stl/
                ${PROJECT_BINARY_DIR}/Output/Server/

        # Copy SDL2 dlls to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/SDL2/
                ${PROJECT_BINARY_DIR}/Output/Server/

        # Copy pthread dll to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/WinPthread/
                ${PROJECT_BINARY_DIR}/Output/Server/

        # Copy the .exe to the output directory.
        TARGET Server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/Server.exe
                ${PROJECT_BINARY_DIR}/Output/Server/
)

# Build all of the subdirectories
add_subdirectory(Launch)
add_subdirectory(Game)
add_subdirectory(Utility)
