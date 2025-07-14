cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine Shared Library")

# Add our static library target.
add_library(SharedLib STATIC "")

# Provide an alias that can be used whether the target comes from 
# add_subdirectory() or find_package().
add_library(AmalgamEngine::SharedLib ALIAS SharedLib)

target_include_directories(SharedLib
    PUBLIC
        ${SDL2_INCLUDE_DIRS}
        ${SDL2PP_INCLUDE_DIRS}
)

target_link_libraries(SharedLib
    PUBLIC
        ${SDL2_LIBRARIES}
        ${SDL2PP_LIBRARIES}
        SDL2_net-static
        SDL2pp
        boost_mp11
        lz4_static
        libmorton::libmorton 
        EnTT::EnTT
        Bitsery::bitsery
        nlohmann_json::nlohmann_json
        Tracy::TracyClient

        # Only included for PCH.
        readerwriterqueue
        QueuedEvents
)

# Compile with C++23.
target_compile_features(SharedLib PRIVATE cxx_std_23)
set_target_properties(SharedLib PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(SharedLib PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(SharedLib PUBLIC /W3 /permissive-)
    # Temp: MSVC needs this to properly set __cplusplus, and we need it to 
    #       be > 2017 for the bitsery extension headers.
    target_compile_options(SharedLib PUBLIC "/Zc:__cplusplus")
endif()

# Build all of the subdirectories.
add_subdirectory(Messages)
add_subdirectory(Network)
add_subdirectory(Simulation)
add_subdirectory(Utility)
