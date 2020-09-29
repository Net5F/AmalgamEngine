target_sources(Shared
    PUBLIC
        Public/ConnectionResponse.h
        Public/Entity.h
        Public/EntityUpdate.h
        Public/MessageTools.h
)

target_include_directories(Shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
