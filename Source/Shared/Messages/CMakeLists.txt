target_sources(Shared
    PRIVATE
        Private/MessageTools.cpp
    PUBLIC
        Public/MessageTools.h
        Public/ConnectionResponse.h
        Public/EntityState.h
        Public/EntityUpdate.h
        Public/Heartbeat.h
        Public/ClientInput.h
)

target_include_directories(Shared
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
