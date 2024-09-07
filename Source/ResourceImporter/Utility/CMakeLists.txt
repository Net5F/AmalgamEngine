target_sources(ResourceImporter
    PRIVATE
        Private/SpriteTools.cpp
    PUBLIC
        Public/SpriteTools.h
)

target_include_directories(ResourceImporter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
