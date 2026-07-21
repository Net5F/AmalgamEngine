cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine AccountServer")

###############################################################################
# Configure Step
###############################################################################
add_executable(AccountServer WIN32 "")

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(AccountServer REUSE_FROM SharedLib)
endif()

target_link_libraries(AccountServer
    PRIVATE
        SDL3::SDL3
        nlohmann_json::nlohmann_json
        SharedLib
        asio
        SQLiteCpp
        sqlite3
)

# Compile with C++23.
target_compile_features(AccountServer PRIVATE cxx_std_23)
set_target_properties(AccountServer PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(AccountServer PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(AccountServer PUBLIC /W3 /permissive-)
endif()

# On Windows, copy the SDL3 DLLs into the build folder so we can run our executable.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET AccountServer POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_RUNTIME_DLLS:AccountServer>
			$<TARGET_FILE_DIR:AccountServer>
		COMMAND_EXPAND_LISTS
	)
endif()

# Build all of the subdirectories
add_subdirectory(Config)
add_subdirectory(DataModel)
add_subdirectory(Launch)
add_subdirectory(Network)

###############################################################################
# Install Step (Packaging the application)
###############################################################################
# Define an install step that copies the executable and all resources.
install(TARGETS AccountServer
        RUNTIME_DEPENDENCY_SET account_server_deps
        RUNTIME DESTINATION ${CMAKE_PROJECT_NAME}/AccountServer)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/AccountServer/Common/
        DESTINATION ${CMAKE_PROJECT_NAME}/AccountServer)

# Add platform-specific files to the install step.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/AccountServer/Windows/
            DESTINATION ${CMAKE_PROJECT_NAME}/AccountServer)

    # On Windows, we provide the SDL3 DLLs.
    install(RUNTIME_DEPENDENCY_SET account_server_deps
		    DESTINATION ${CMAKE_PROJECT_NAME}/AccountServer)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/AccountServer/Linux/
            DESTINATION ${CMAKE_PROJECT_NAME}/AccountServer)
endif()
