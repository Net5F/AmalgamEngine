target_sources(SpriteEditor
    PRIVATE
        Private/Paths.cpp
    PUBLIC
        Public/Config.h
        Public/Paths.h
)

target_include_directories(SpriteEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
