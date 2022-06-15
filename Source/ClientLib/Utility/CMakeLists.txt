target_sources(ClientLib
    PRIVATE
        Private/ClientTransforms.cpp
        Private/SpriteData.cpp
    PUBLIC
        Public/ClientTransforms.h
        Public/SpriteData.h
)

target_include_directories(ClientLib
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
