target_sources(SpriteEditor
    PRIVATE
        Private/SpriteData.cpp
    PUBLIC
        Public/SpriteData.h
)

target_include_directories(SpriteEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
