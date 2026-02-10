cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine ResourceImporter")

###############################################################################
# Configure Step
###############################################################################
add_executable(ResourceImporter WIN32 "")

target_include_directories(ResourceImporter
    PRIVATE
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_GFX_INCLUDE_DIRS} ${SDL2PP_INCLUDE_DIRS}
)

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(ResourceImporter REUSE_FROM SharedLib)
endif()

target_link_libraries(ResourceImporter
    PRIVATE
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2_GFX_LIBRARIES} ${SDL2PP_LIBRARIES}
        AmalgamUI
        EnTT::EnTT
        nfd
        nlohmann_json::nlohmann_json
        SharedLib
)

# Compile with C++23.
target_compile_features(ResourceImporter PRIVATE cxx_std_23)
set_target_properties(ResourceImporter PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ResourceImporter PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(ResourceImporter PUBLIC /W3 /permissive-)
endif()

# Copy assets to the build directory.
file(COPY ${PROJECT_SOURCE_DIR}/Resources/ResourceImporter/Common/Assets/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Assets/)

# On Windows, copy the SDL2 DLLs into the build folder so we can run our executable.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SDL2_DLL_PATHS 
         ${SDL2_DIR}/lib/x64/
         ${SDL2_IMAGE_INCLUDE_DIR}/../lib/x64
         ${SDL2_MIXER_INCLUDE_DIR}/../lib/x64
         ${SDL2_TTF_INCLUDE_DIR}/../lib/x64
         ${SDL2_GFX_INCLUDE_DIR}/../lib/x64
    )

    foreach(DLL_PATH ${SDL2_DLL_PATHS})
        file(GLOB SDL2_DLLS ${DLL_PATH}/*.dll)
        file(COPY ${SDL2_DLLS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
    endforeach()
endif()

# Build all of the subdirectories
add_subdirectory(Config)
add_subdirectory(DataModel)
add_subdirectory(Launch)
add_subdirectory(Renderer)
add_subdirectory(UserInterface)
add_subdirectory(Utility)

###############################################################################
# Install Step (Packaging the application)
###############################################################################
# Define an install step that copies the executable and all resources.
install(TARGETS ResourceImporter RUNTIME DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/ResourceImporter/Common/
        DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter)

# Add platform-specific files to the install step.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/ResourceImporter/Windows/
            DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter)

    # On Windows, we provide the SDL2 DLLs.
    foreach(DLL_PATH ${SDL2_DLL_PATHS})
        install(DIRECTORY ${DLL_PATH}/
                DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter
                FILES_MATCHING PATTERN "*.dll")
    endforeach()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/ResourceImporter/Linux/
            DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter)
endif()
