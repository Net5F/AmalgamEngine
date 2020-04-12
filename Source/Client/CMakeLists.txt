add_executable(Client "")

target_include_directories(Client
    PRIVATE
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_NET_INCLUDE_DIRS}
)

target_link_libraries(Client
    PRIVATE
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2_NET_LIBRARIES}
        SDL2pp
        Messend
        flatbuffers
        Shared
)

# Compile with C++17
target_compile_features(Client PRIVATE cxx_std_17)
set_target_properties(Client PROPERTIES CXX_EXTENSIONS OFF)

add_custom_command(
        TARGET Client POST_BUILD
        
        # Make sure the output directory exists
        COMMAND ${CMAKE_COMMAND} -E make_directory 
                ${PROJECT_BINARY_DIR}/Output/Client/

        # Copy C and C++ stl dlls to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/Stl/
                ${PROJECT_BINARY_DIR}/Output/Client/

        # Copy SDL2 dlls to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Libraries/SDL2/
                ${PROJECT_BINARY_DIR}/Output/Client/

        # Copy resource files to the output directory.
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_SOURCE_DIR}/../Resources/
                ${PROJECT_BINARY_DIR}/Output/Client/Resources/

        # Copy the .exe to the output directory.
        TARGET Client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_BINARY_DIR}/Client.exe
                ${PROJECT_BINARY_DIR}/Output/Client/
)

# Build all of the subdirectories
add_subdirectory(Launch)
add_subdirectory(Game)
add_subdirectory(Graphics)
