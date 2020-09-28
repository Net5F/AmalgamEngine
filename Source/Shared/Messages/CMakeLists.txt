target_sources(Shared
    PRIVATE
        Private/MessageUtil.cpp
    PUBLIC
        Public/ConnectionResponse.h
        Public/Entity.h
        Public/EntityUpdate.h
)

target_include_directories(Shared
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
