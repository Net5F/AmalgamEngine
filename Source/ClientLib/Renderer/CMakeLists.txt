target_sources(ClientLib
    PRIVATE
        Private/Renderer.cpp
        Private/WorldSpriteSorter.cpp
    PUBLIC
        Public/IRendererExtension.h
        Public/Renderer.h
        Public/RendererContext.h
        Public/SpriteSortInfo.h
        Public/WorldSpriteSorter.h
)

target_include_directories(ClientLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
