cmake_minimum_required(VERSION 3.13)

project(Amalgam)

# Tell CMake where the CMake folder is, since our source dir isn't the base dir.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../CMake/")

# Make ThreadSanitizer available. Will only be used if AM_THREADSANITIZER == ON.
include("${CMAKE_MODULE_PATH}/ThreadSanitizer.cmake")

###############################################################################
# Dependencies
###############################################################################
# Find SDL2 and associated libs.
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_gfx REQUIRED)
include("${PROJECT_SOURCE_DIR}/../CMake/SDL2_net.cmake")

# Configure static library SDL2pp.
set(SDL2PP_WITH_IMAGE ON)
set(SDL2PP_WITH_MIXER ON)
set(SDL2PP_WITH_TTF ON)
set(SDL2PP_WITH_NET ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/libSDL2pp/"
                 "${PROJECT_BINARY_DIR}/Libraries/libSDL2pp/")

# Configure bitsery.
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/bitsery/"
                 "${PROJECT_BINARY_DIR}/Libraries/bitsery/")

# Configure Remotery.
set(ENABLE_PROFILING OFF CACHE BOOL "Enable Remotery profiling." FORCE)
include("${PROJECT_SOURCE_DIR}/../CMake/Remotery.cmake")

# Configure nlohmann json.
set(JSON_MultipleHeaders ON CACHE INTERNAL "Enable forward declaration.")
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/json/"
                 "${PROJECT_BINARY_DIR}/Libraries/json/")
                 
# Configure nativefiledialog-extended.
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/nativefiledialog-extended/"
                 "${PROJECT_BINARY_DIR}/Libraries/nativefiledialog-extended/")

# Configure header-only readerwriterqueue.
include("${PROJECT_SOURCE_DIR}/../CMake/readerwriterqueue.cmake")

# Configure header-only CircularBuffer.
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/CircularBuffer/"
                 "${PROJECT_BINARY_DIR}/Libraries/CircularBuffer/")

# Configure header-only QueuedEvents.
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/QueuedEvents/"
                 "${PROJECT_BINARY_DIR}/Libraries/QueuedEvents/")

# Configure header-only EnTT.
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/entt/"
                 "${PROJECT_BINARY_DIR}/Libraries/entt/")

# Configure AmalgamUI.
add_subdirectory("${PROJECT_SOURCE_DIR}/../Libraries/AmalgamUI/"
                 "${PROJECT_BINARY_DIR}/Libraries/AmalgamUI/")

###############################################################################
# Modules
###############################################################################
# Configure shared library
add_subdirectory(Shared)

# Configure client
add_subdirectory(Client)

# Configure server
add_subdirectory(Server)

# Configure sprite editor
add_subdirectory(SpriteEditor)

# Configure tests.
# (No fancy flags since we aren't yet a lib, just uncomment when you want to build tests.)
add_subdirectory(Tests)

# Configure tools.
add_subdirectory(Tools)
