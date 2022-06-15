target_sources(ClientLib
    PRIVATE
        Private/Renderer.cpp
        Private/WorldSpritePreparer.cpp
    PUBLIC
        Public/Renderer.h
        Public/SpriteRenderInfo.h
        Public/WorldSpritePreparer.h
)

target_include_directories(ClientLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
