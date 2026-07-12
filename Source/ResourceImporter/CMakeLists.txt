cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine ResourceImporter")

###############################################################################
# Configure Step
###############################################################################
add_executable(ResourceImporter WIN32 "")

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(ResourceImporter REUSE_FROM SharedLib)
endif()

target_link_libraries(ResourceImporter
    PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        SDL3_mixer::SDL3_mixer
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

# On Windows, copy the SDL3 DLLs into the build folder so we can run our executable.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET ResourceImporter POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_RUNTIME_DLLS:ResourceImporter>
			$<TARGET_FILE_DIR:ResourceImporter>
		COMMAND_EXPAND_LISTS
	)
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
install(TARGETS ResourceImporter
        RUNTIME_DEPENDENCY_SET resource_importer_deps
        RUNTIME DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/ResourceImporter/Common/
        DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter)

# Add platform-specific files to the install step.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/ResourceImporter/Windows/
            DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter)

    # On Windows, we provide the SDL3 DLLs.
    install(RUNTIME_DEPENDENCY_SET resource_importer_deps
		    DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/ResourceImporter/Linux/
            DESTINATION ${CMAKE_PROJECT_NAME}/ResourceImporter)
endif()
