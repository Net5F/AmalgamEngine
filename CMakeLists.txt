cmake_minimum_required(VERSION 3.13)

project(AmalgamEngine)

# Tell CMake where the CMake folder is.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")

###############################################################################
# Options
###############################################################################
set(AM_CONFIG_OVERRIDE_PATH
    "" CACHE PATH
    "Override the engine's default config files with the file at the provided path.")

option(AM_BUILD_SPRITE_EDITOR "Build Amalgam Engine Sprite Editor." OFF)

option(AM_BUILD_TESTS "Build Amalgam Engine tests." OFF)

###############################################################################
# Dependencies
###############################################################################
# On Windows, if the user hasn't provided paths to the SDL libs, download them.
include(FetchContent)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (NOT DEFINED SDL2_DIR)
        message(STATUS "Downloading dependency: SDL2")
        FetchContent_Declare(SDL2Download
            URL https://www.libsdl.org/release/SDL2-devel-2.0.20-VC.zip
            URL_HASH MD5=85fe721cb9aa42fb6703da6f30b5df45
        )
        FetchContent_MakeAvailable(SDL2Download)
        set(SDL2_DIR ${sdl2download_SOURCE_DIR} CACHE STRING "")
        set(SDL2_INCLUDE_DIR ${sdl2download_SOURCE_DIR}/include CACHE STRING "")
    endif()
    if (NOT DEFINED SDL2_IMAGE_LIBRARY)
        message(STATUS "Downloading dependency: SDL2_Image")
        FetchContent_Declare(SDL2ImageDownload
            URL https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.5-VC.zip
            URL_HASH MD5=b6c8c85b64e01b7096364036438b3333
        )
        FetchContent_MakeAvailable(SDL2ImageDownload)
        set(SDL2_IMAGE_LIBRARY ${sdl2imagedownload_SOURCE_DIR}/lib/x64/SDL2_image.lib CACHE STRING "")
        set(SDL2_IMAGE_INCLUDE_DIR ${sdl2imagedownload_SOURCE_DIR}/include CACHE STRING "")
    endif()
    if (NOT DEFINED SDL2_MIXER_LIBRARY)
        message(STATUS "Downloading dependency: SDL2_Mixer")
        FetchContent_Declare(SDL2MixerDownload
            URL https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-2.0.4-VC.zip
            URL_HASH MD5=9fd36fba973a7011b9e9a1fa5193a53a
        )
        FetchContent_MakeAvailable(SDL2MixerDownload)
        set(SDL2_MIXER_LIBRARY ${sdl2mixerdownload_SOURCE_DIR}/lib/x64/SDL2_mixer.lib CACHE STRING "")
        set(SDL2_MIXER_INCLUDE_DIR ${sdl2mixerdownload_SOURCE_DIR}/include CACHE STRING "")
    endif()
    if (NOT DEFINED SDL2_TTF_LIBRARY)
        message(STATUS "Downloading dependency: SDL2_TTF")
        FetchContent_Declare(SDL2TtfDownload
            URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.20.1/SDL2_ttf-devel-2.20.1-VC.zip
            URL_HASH MD5=0e93c9052d01eebb9c18b1e641322637
        )
        FetchContent_MakeAvailable(SDL2TtfDownload)
        set(SDL2_TTF_LIBRARY ${sdl2ttfdownload_SOURCE_DIR}/lib/x64/SDL2_ttf.lib CACHE STRING "")
        set(SDL2_TTF_INCLUDE_DIR ${sdl2ttfdownload_SOURCE_DIR}/include CACHE STRING "")
    endif()
    if (NOT DEFINED SDL2_GFX_LIBRARY)
        message(STATUS "Downloading dependency: SDL2_GFX")
        FetchContent_Declare(SDL2GfxDownload
            URL https://github.com/Net5F/SDL_gfx/releases/download/1.0.4/SDL2_gfx-devel-1.0.4.zip
            URL_HASH MD5=381bc9fd5e7052e98ce5710893a31304
        )
        FetchContent_MakeAvailable(SDL2GfxDownload)
        set(SDL2_GFX_LIBRARY ${sdl2gfxdownload_SOURCE_DIR}/lib/x64/SDL2_gfx.lib CACHE STRING "")
        set(SDL2_GFX_INCLUDE_DIR ${sdl2gfxdownload_SOURCE_DIR}/include CACHE STRING "")
    endif()
endif()

# Find SDL2 and associated libs.
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_gfx REQUIRED)
include("${PROJECT_SOURCE_DIR}/CMake/SDL2_net.cmake")

# Configure lz4.
option(LZ4_BUILD_CLI "Build lz4 program" OFF)
option(LZ4_BUILD_LEGACY_LZ4C "Build lz4c program with legacy argument support" OFF)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/lz4/build/cmake/"
                 "${PROJECT_BINARY_DIR}/Libraries/lz4/")

# Configure SDL2pp.
set(SDL2PP_WITH_IMAGE ON)
set(SDL2PP_WITH_MIXER ON)
set(SDL2PP_WITH_TTF ON)
set(SDL2PP_WITH_NET ON)
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/libSDL2pp/"
                 "${PROJECT_BINARY_DIR}/Libraries/libSDL2pp/")

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
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/tracy/"
                 "${PROJECT_BINARY_DIR}/Libraries/tracy/"
                 EXCLUDE_FROM_ALL)

# Configure nlohmann json.
# Note: We use FetchContent because it's a much smaller download (100KB vs 300MB).
message(STATUS "Downloading dependency if not present: nlohmann_json")
set(JSON_MultipleHeaders ON CACHE INTERNAL "Enable forward declaration.")
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
    URL_HASH MD5=4b67aba51ddf17c798e80361f527f50e
)
FetchContent_MakeAvailable(json)
                 
# Configure nativefiledialog-extended.
add_subdirectory("${PROJECT_SOURCE_DIR}/Libraries/nativefiledialog-extended/"
                 "${PROJECT_BINARY_DIR}/Libraries/nativefiledialog-extended/"
                 EXCLUDE_FROM_ALL)

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
