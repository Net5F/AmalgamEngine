cmake_minimum_required(VERSION 3.5)

message(STATUS "Configuring Lua")

# Add our static library target
add_library(Lua STATIC
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lapi.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lcode.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lctype.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/ldebug.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/ldo.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/ldump.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lfunc.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lgc.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/llex.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lmem.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lobject.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lopcodes.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lparser.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lstate.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lstring.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/ltable.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/ltm.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lundump.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lvm.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lzio.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lauxlib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lbaselib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lcorolib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/ldblib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/liolib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lmathlib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/loslib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lstrlib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/ltablib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/lutf8lib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/loadlib.c
      ${PROJECT_SOURCE_DIR}/Libraries/lua/linit.c
)

target_include_directories(Lua
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Libraries/lua
)
