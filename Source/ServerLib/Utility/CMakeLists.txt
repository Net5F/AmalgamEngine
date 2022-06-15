target_sources(ServerLib
    PRIVATE
        Private/SpriteData.cpp
    PUBLIC
        Public/SpriteData.h
)

target_include_directories(ServerLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
