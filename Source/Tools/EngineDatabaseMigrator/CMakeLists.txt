cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring EngineDatabaseMigrator")

add_executable(EngineDatabaseMigrator
    Private/EngineDatabaseMigratorMain.cpp
    Private/MigrateEngineComponents.cpp
    Private/MigrateEngineComponents.h
    Private/MigrationRunner.cpp
    Private/MigrationRunner.h
)

target_include_directories(EngineDatabaseMigrator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(EngineDatabaseMigrator
    PRIVATE
        AmalgamEngine::SharedLib
        AmalgamEngine::ServerLib
        SQLiteCpp
        sqlite3
)

# Compile with C++23.
target_compile_features(EngineDatabaseMigrator PRIVATE cxx_std_23)
set_target_properties(EngineDatabaseMigrator PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(EngineDatabaseMigrator PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(EngineDatabaseMigrator PUBLIC /W3 /permissive-)
endif()
