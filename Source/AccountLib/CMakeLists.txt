cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine Account Library")

# Add our static library target.
add_library(AccountLib STATIC "")

# Provide an alias that can be used whether the target comes from 
# add_subdirectory() or find_package().
add_library(AmalgamEngine::AccountLib ALIAS AccountLib)

target_link_libraries(AccountLib
    PUBLIC
        SDL3::SDL3
)

# Compile with C++23.
target_compile_features(AccountLib PRIVATE cxx_std_23)
set_target_properties(AccountLib PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(AccountLib PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(AccountLib PUBLIC /W3 /permissive-)
    # Temp: MSVC needs this to properly set __cplusplus, and we need it to 
    #       be > 2017 for the bitsery extension headers.
    target_compile_options(AccountLib PUBLIC "/Zc:__cplusplus")
endif()

# Build all of the subdirectories.
add_subdirectory(Account)
add_subdirectory(Messages)
