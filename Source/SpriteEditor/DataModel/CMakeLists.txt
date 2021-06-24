target_sources(SpriteEditor
    PRIVATE
        Private/SpriteDataModel.cpp
    PUBLIC
        Public/SpriteDataModel.h
        Public/SpriteSheet.h
        Public/SpriteStaticData.h
)

target_include_directories(SpriteEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
