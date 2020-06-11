cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring SDL_mixer")

# FIXME: missing CMakeLists.txt for MPG123
set(SUPPORT_MP3_MPG123 OFF CACHE BOOL "" FORCE)

option(SUPPORT_FLAC "Support loading FLAC music with libFLAC" OFF)
option(SUPPORT_OGG "Support loading OGG Vorbis music via Tremor" OFF)
option(SUPPORT_MP3_MPG123 "Support loading MP3 music via MPG123" OFF)
option(SUPPORT_MOD_MODPLUG "Support loading MOD music via modplug" OFF)
option(SUPPORT_MID_TIMIDITY "Support TiMidity" OFF)

# Add our static library target
add_library(SDL2_mixer-static STATIC
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/effect_position.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/effects_internal.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/effect_stereoreverse.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/mixer.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/music.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/compat.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/load_aiff.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/load_voc.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_cmd.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_flac.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_fluidsynth.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_mad.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_mikmod.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_modplug.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_mpg123.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_nativemidi.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_ogg.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_opus.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_timidity.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs/music_wav.c
)
set_target_properties(SDL2_mixer-static PROPERTIES OUTPUT_NAME "SDL2_mixer")

if (SUPPORT_FLAC)
    add_definitions(-DMUSIC_FLAC)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/external/flac-1.3.3)
    include_directories(${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/external/flac-1.3.3/include)
    target_link_libraries(SDL2_mixer PRIVATE FLAC)
endif()

if (SUPPORT_OGG)
    add_definitions(-DMUSIC_OGG -DOGG_USE_TREMOR -DOGG_HEADER=<ivorbisfile.h>)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/external/libogg-1.3.2)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/external/libvorbisidec-1.2.1)
    include_directories(${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/external/libvorbisidec-1.2.1)
    target_link_libraries(SDL2_mixer PRIVATE vorbisfile vorbisidec ogg)
endif()

if (SUPPORT_MP3_MPG123)
    add_definitions(-DMUSIC_MP3_MPG123)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/external/mpg123-1.25.13)
    target_link_libraries(SDL2_mixer PRIVATE mpg123)
endif()

if (SUPPORT_MOD_MODPLUG)
    add_definitions(-DMUSIC_MOD_MODPLUG -DMODPLUG_HEADER=<modplug.h>)
    add_subdirectory(${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/external/libmodplug-0.8.9.0)
    include_directories(${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/external/libmodplug-0.8.9.0/src)
    target_link_libraries(SDL2_mixer PRIVATE modplug)
endif()

if (SUPPORT_MID_TIMIDITY)
    add_definitions(-DMUSIC_MID_TIMIDITY)
    add_subdirectory(timidity)
    target_link_libraries(SDL2_mixer PRIVATE timidity)
endif()

target_include_directories(SDL2_mixer-static
    PRIVATE
        ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src
        ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/src/codecs
    PUBLIC
        ${PROJECT_SOURCE_DIR}/../Libraries/SDL_mixer/include
)

target_link_libraries(SDL2_mixer-static PRIVATE SDL2-static)
