cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring SDL_net")

# Add our static library target
add_library(SDL2_net-static STATIC
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_net/SDLnet.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_net/SDLnetTCP.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_net/SDLnetUDP.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_net/SDLnetselect.c
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_net/SDLnetsys.h
       ${PROJECT_SOURCE_DIR}/../Libraries/SDL_net/SDL_net.h
)
set_target_properties(SDL2_net-static PROPERTIES OUTPUT_NAME "SDL2_net")

target_include_directories(SDL2_net-static
    PUBLIC
        ${PROJECT_SOURCE_DIR}/../Libraries/SDL_net
)

target_link_libraries(SDL2_net-static PRIVATE SDL2-static)

if(MINGW)
    target_link_libraries(SDL2_net-static PRIVATE ws2_32 iphlpapi)
endif()
