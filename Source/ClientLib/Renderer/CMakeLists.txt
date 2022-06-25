target_sources(ClientLib
    PRIVATE
        Private/Renderer.cpp
        Private/WorldSpritePreparer.cpp
    PUBLIC
        Public/IRendererExtension.h
        Public/Renderer.h
        Public/RendererExDependencies.h
        Public/SpriteRenderInfo.h
        Public/WorldSpritePreparer.h
)

target_include_directories(ClientLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
