cmake_minimum_required(VERSION 3.13)

project(Amalgam)

# Tell CMake where the CMake folder is, since our source dir isn't the base dir.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../CMake/")

# Find SDL2 and associated libs
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_net REQUIRED)

# Build static library dependency SDL2pp
set(SDL2PP_WITH_IMAGE ON)
set(SDL2PP_WITH_MIXER ON)
set(SDL2PP_WITH_TTF ON)
set(SDL2PP_WITH_NET ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/libSDL2pp/"
                 "${PROJECT_BINARY_DIR}/Libraries/libSDL2pp/")

# Build flatbuffers
SET(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "Enable the build of tests and samples." FORCE)
SET(FLATBUFFERS_INSTALL OFF CACHE BOOL "Enable the installation of targets." FORCE)
SET(FLATBUFFERS_BUILD_FLATHASH OFF CACHE BOOL "Enable the build of flathash" FORCE)
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/flatbuffers/"
                 "${PROJECT_BINARY_DIR}/Libraries/flatbuffers/")

# Pack readerwriterqueue headers into a target
include("${PROJECT_SOURCE_DIR}/../CMake/readerwriterqueue.cmake")

# Build shared library
add_subdirectory(Shared)

# Build client
add_subdirectory(Client)

# Build server
add_subdirectory(Server)

# Build editor
#add_subdirectory(Editor)
