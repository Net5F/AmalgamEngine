cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine ChatServer")

###############################################################################
# Configure Step
###############################################################################
add_executable(ChatServer "")

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(ChatServer REUSE_FROM SharedLib)
endif()

target_link_libraries(ChatServer
    PRIVATE
        SDL3::SDL3
        nlohmann_json::nlohmann_json
        SharedLib
        asio
)

# Compile with C++23.
target_compile_features(ChatServer PRIVATE cxx_std_23)
set_target_properties(ChatServer PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(ChatServer PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(ChatServer PUBLIC /W3 /permissive-)
endif()

# If any of our needed files don't exist in the build directory, copy them.
# Note: We don't copy every time, since it would overwrite any changes that
#       we hadn't copied to the Resources directory yet.
include(${AmalgamEngine_SOURCE_DIR}/CMake/copy_if_does_not_exist.cmake)
copy_if_does_not_exist("Resources/ChatServer/Common/UserConfig.json")

# Copy local TLS credentials to the build directory.
file(COPY
    "${AmalgamEngine_SOURCE_DIR}/TlsCredentials/Local/chat-server.crt"
    "${AmalgamEngine_SOURCE_DIR}/TlsCredentials/Local/chat-server.key"
    "${AmalgamEngine_SOURCE_DIR}/TlsCredentials/Local/account-server.pin"
    "${AmalgamEngine_SOURCE_DIR}/TlsCredentials/Local/world-server.pin"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# On Windows, copy runtime DLLs into the build folder so we can run our executable.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET ChatServer POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_RUNTIME_DLLS:ChatServer>
			${AM_OPENSSL_RUNTIME_DLLS}
			$<TARGET_FILE_DIR:ChatServer>
		COMMAND_EXPAND_LISTS
	)
endif()

# Build all of the subdirectories
add_subdirectory(Config)
add_subdirectory(Launch)
add_subdirectory(Network)

###############################################################################
# Install Step (Packaging the application)
###############################################################################
# Define an install step that copies the executable and all resources.
set(CHAT_SERVER_REMOTE_TLS_PINS
    "${CMAKE_SOURCE_DIR}/TlsCredentials/Remote/account-server.pin"
    "${CMAKE_SOURCE_DIR}/TlsCredentials/Remote/world-server.pin")
include("${AmalgamEngine_SOURCE_DIR}/CMake/am_check_tls_pins_exist.cmake")
am_check_tls_pins_exist(${CHAT_SERVER_REMOTE_TLS_PINS})
install(FILES ${CHAT_SERVER_REMOTE_TLS_PINS}
        DESTINATION ${CMAKE_PROJECT_NAME}/ChatServer)

install(TARGETS ChatServer
        RUNTIME DESTINATION ${CMAKE_PROJECT_NAME}/ChatServer)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/Resources/ChatServer/Common/
        DESTINATION ${CMAKE_PROJECT_NAME}/ChatServer)

# Add platform-specific files to the install step.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # On Windows, provide the SDL3 DLLs.
    install(FILES
            ${AM_SDL3_RUNTIME_DLL}
            ${AM_SDL3_IMAGE_RUNTIME_DLL}
            DESTINATION ${CMAKE_PROJECT_NAME}/ChatServer)

    # FindOpenSSL doesn't expose its runtime DLLs as imported targets.
    install(FILES ${AM_OPENSSL_RUNTIME_DLLS}
            DESTINATION ${CMAKE_PROJECT_NAME}/ChatServer)
endif()
