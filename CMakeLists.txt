cmake_minimum_required(VERSION 3.24)

# Temp: Set to new behavior to avoid warning.
cmake_policy(SET CMP0135 NEW)

project(AmalgamEngine)

# Tell CMake where the CMake folder is.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")

###############################################################################
# Options
###############################################################################
option(AM_BUILD_RESOURCE_IMPORTER "Build Amalgam Engine Resource Importer." OFF)

option(AM_BUILD_TOOLS "Build Amalgam Engine tools." OFF)

option(AM_BUILD_TESTS "Build Amalgam Engine tests." OFF)

###############################################################################
# Dependencies
###############################################################################
# On Windows, if the user hasn't provided paths to the SDL libs, download them.
include(FetchContent)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (NOT DEFINED SDL3_DIR)
        message(STATUS "Downloading dependency: SDL3")
        FetchContent_Declare(SDL3Download
            URL https://www.libsdl.org/release/SDL3-devel-3.4.2-VC.zip
            URL_HASH MD5=24e189f730969a8f388e676451cf635f
        )
        FetchContent_MakeAvailable(SDL3Download)
		list(APPEND CMAKE_PREFIX_PATH ${sdl3download_SOURCE_DIR})
    endif()
    if (NOT DEFINED SDL3_IMAGE_LIBRARY)
        message(STATUS "Downloading dependency: SDL3_Image")
        FetchContent_Declare(SDL3ImageDownload
            URL https://www.libsdl.org/projects/SDL_image/release/SDL3_image-devel-3.4.0-VC.zip
            URL_HASH MD5=5abf9939f7ede48f41c51d94aee77052
        )
        FetchContent_MakeAvailable(SDL3ImageDownload)
		list(APPEND CMAKE_PREFIX_PATH ${sdl3imagedownload_SOURCE_DIR})
    endif()
    if (NOT DEFINED SDL3_MIXER_LIBRARY)
        message(STATUS "Downloading dependency: SDL3_Mixer")
        FetchContent_Declare(SDL3MixerDownload
            URL https://www.libsdl.org/projects/SDL_mixer/release/SDL3_mixer-devel-3.1.2-VC.zip
            URL_HASH MD5=683d1bc8e644e446ffca3442717dc2d3
        )
        FetchContent_MakeAvailable(SDL3MixerDownload)
		list(APPEND CMAKE_PREFIX_PATH ${sdl3mixerdownload_SOURCE_DIR})
    endif()
    if (NOT DEFINED SDL3_TTF_LIBRARY)
        message(STATUS "Downloading dependency: SDL3_TTF")
        FetchContent_Declare(SDL3TtfDownload
            URL https://www.libsdl.org/projects/SDL_ttf/release/SDL3_ttf-devel-3.2.2-VC.zip
            URL_HASH MD5=a36475472f285978c38475994714d762
        )
        FetchContent_MakeAvailable(SDL3TtfDownload)
		list(APPEND CMAKE_PREFIX_PATH ${sdl3ttfdownload_SOURCE_DIR})
    endif()
endif()

# Find SDL3 and associated libs.
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(SDL3_ttf REQUIRED)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/SDL_net/"
                 "${PROJECT_BINARY_DIR}/Libraries/SDL_net/")

# Configure lz4.
option(LZ4_BUILD_CLI "Build lz4 program" OFF)
option(LZ4_BUILD_LEGACY_LZ4C "Build lz4c program with legacy argument support" OFF)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/lz4/build/cmake/"
                 "${PROJECT_BINARY_DIR}/Libraries/lz4/")

# Configure Boost MP11.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/mp11/"
                 "${PROJECT_BINARY_DIR}/Libraries/mp11/")

# Configure bitsery.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/bitsery/"
                 "${PROJECT_BINARY_DIR}/Libraries/bitsery/"
                 EXCLUDE_FROM_ALL)

# Configure Tracy.
option(TRACY_ENABLE "Enable Tracy profiling." OFF)
option(TRACY_ON_DEMAND "Wait for connection before starting profiling." ON)
# (Disabled because it requires extra dependencies. Can enable if useful.)
option(TRACY_NO_CALLSTACK "Disable all callstack related functionality" ON)
# (Below 2 are disabled because they didn't seem useful. Can enable if useful.)
option(TRACY_NO_VSYNC_CAPTURE "Disable capture of hardware Vsync events" ON)
option(TRACY_NO_FRAME_IMAGE  "Disable the frame image support and its thread" ON)
# (Disabled because it uses a lot of CPU in the remote server.)
option(TRACY_NO_SYSTEM_TRACING  "Disable systrace sampling" ON)
# (Disabled because the remote server doesn't support the default timers.)
option(TRACY_TIMER_FALLBACK "Use lower resolution timers" ON)
# (Disabled because it was failing to compile.)
option(TRACY_NO_CRASH_HANDLER "Disable crash handling" ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/tracy/"
                 "${PROJECT_BINARY_DIR}/Libraries/tracy/"
                 EXCLUDE_FROM_ALL)

# Configure nlohmann json.
# Note: We use FetchContent because it's a much smaller download (100KB vs 300MB).
message(STATUS "Downloading dependency if not present: nlohmann_json")
set(JSON_MultipleHeaders ON CACHE INTERNAL "Enable forward declaration.")
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH MD5=e155202b2a589137f6804724bd182f12
)
FetchContent_MakeAvailable(json)
                 
# Configure nativefiledialog-extended.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/nativefiledialog-extended/"
                 "${PROJECT_BINARY_DIR}/Libraries/nativefiledialog-extended/"
                 EXCLUDE_FROM_ALL)

# Configure Lua.
include("${PROJECT_SOURCE_DIR}/CMake/lua.cmake")

# Configure Sol2.
option(SOL2_SYSTEM_INCLUDE "Sol2 as system include" OFF)
option(SOL2_ENABLE_INSTALL "Enable installation of Sol2" OFF)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/sol2/"
                 "${PROJECT_BINARY_DIR}/Libraries/sol2/")

# Configure SQLiteCpp.
option(SQLITECPP_INCLUDE_SCRIPT "Include config & script files." OFF)
option(SQLITE_ENABLE_ASSERT_HANDLER "Enable the user definition of a assertion_failed() handler." ON)
option(SQLITECPP_RUN_CPPLINT "Run cpplint.py tool for Google C++ StyleGuide." OFF)
option(SQLITECPP_RUN_CPPCHECK "Run cppcheck C++ static analysis tool." OFF)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/SQLiteCpp/"
                 "${PROJECT_BINARY_DIR}/Libraries/SQLiteCpp/" EXCLUDE_FROM_ALL)

# Configure libmorton.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/libmorton/"
                 "${PROJECT_BINARY_DIR}/Libraries/libmorton/" EXCLUDE_FROM_ALL)

# Configure header-only readerwriterqueue.
include("${PROJECT_SOURCE_DIR}/CMake/readerwriterqueue.cmake")

# Configure header-only CircularBuffer.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/CircularBuffer/"
                 "${PROJECT_BINARY_DIR}/Libraries/CircularBuffer/")

# Configure header-only QueuedEvents.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/QueuedEvents/"
                 "${PROJECT_BINARY_DIR}/Libraries/QueuedEvents/")

# Configure header-only EnTT.
option(ENTT_INCLUDE_HEADERS "Add all EnTT headers to the EnTT target." ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/entt/"
                 "${PROJECT_BINARY_DIR}/Libraries/entt/"
                 EXCLUDE_FROM_ALL)

# Configure AmalgamUI.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/AmalgamUI/"
                 "${PROJECT_BINARY_DIR}/Libraries/AmalgamUI/")

###############################################################################
# Source
###############################################################################
# Configure our project source.
add_subdirectory(Source)
