target_sources(SharedLib
    PUBLIC
        Public/EntityInteractionType.h
        Public/ItemInteractionType.h
        Public/ItemProperties.h
        Public/SharedConfig.h
        Public/TypeLists/ProjectReplicatedComponentTypes.h
)

target_include_directories(SharedLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
        ${CMAKE_CURRENT_SOURCE_DIR}/Public/TypeLists
)
