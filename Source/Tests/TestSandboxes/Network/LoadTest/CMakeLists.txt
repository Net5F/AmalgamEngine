cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine Load Test Client")

# Load test client
add_executable(LoadTestClient
    Private/LoadTestClientMain.cpp
    Private/NetworkSimulation.cpp
    Public/NetworkSimulation.h
    Private/SimulatedClient.cpp
    Public/SimulatedClient.h
    Private/WorldSimulation.cpp
    Public/WorldSimulation.h
    
    # Client objects
    ${PROJECT_SOURCE_DIR}/Source/EngineSupplementExamples/Client/Config/Public/Config.h
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Config/Private/UserConfig.cpp
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Config/Public/UserConfig.h
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Public/ConnectionError.h
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Private/MessageProcessor.cpp
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Public/MessageProcessor.h
    ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Public/IMessageProcessorExtension.h
)

target_include_directories(LoadTestClient
    PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2PP_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
        ${CMAKE_CURRENT_SOURCE_DIR}/Public

        # Client objects
        ${PROJECT_SOURCE_DIR}/Source/EngineSupplementExamples/Client/Config/Public
        ${PROJECT_SOURCE_DIR}/Source/ClientLib/Config/Public
        ${PROJECT_SOURCE_DIR}/Source/ClientLib/Network/Public
)

# Inherit Shared's precompiled header.
# CMake causes issues when using precompiled headers with GCC on macOS,
# so precompiled headers are disabled for that target.
if ((NOT APPLE) OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang"))
    target_precompile_headers(LoadTestClient REUSE_FROM SharedLib)
endif()

target_link_libraries(LoadTestClient
    PRIVATE
        SharedLib
        readerwriterqueue
        EnTT::EnTT
        QueuedEvents
)

# Compile with C++23.
target_compile_features(LoadTestClient PRIVATE cxx_std_23)
set_target_properties(LoadTestClient PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(LoadTestClient PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(LoadTestClient PUBLIC /W3 /permissive-)
endif()

# Copy UserConfig.json to the build directory.
file(COPY ${CMAKE_SOURCE_DIR}/Resources/Client/Common/UserConfig.json
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# On Windows, copy the SDL3 DLL into the build folder so we can run our executable.
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_custom_command(TARGET LoadTestClient POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_RUNTIME_DLLS:LoadTestClient>
			$<TARGET_FILE_DIR:LoadTestClient>
		COMMAND_EXPAND_LISTS
	)
endif()
