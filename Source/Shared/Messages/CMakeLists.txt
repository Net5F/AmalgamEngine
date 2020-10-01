target_sources(Shared
    PUBLIC
        Public/MessageTools.h
        Public/ConnectionResponse.h
        Public/Entity.h
        Public/EntityUpdate.h
)

target_include_directories(Shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
