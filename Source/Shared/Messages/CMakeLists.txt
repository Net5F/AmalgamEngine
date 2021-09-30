target_sources(Shared
    PUBLIC
        Public/ChunkWireSnapshot.h
        Public/ClientInput.h
        Public/ConnectionResponse.h
        Public/EntityState.h
        Public/EntityUpdate.h
        Public/ExplicitConfirmation.h
        Public/Heartbeat.h
        Public/MessageType.h
        Public/UpdateChunks.h
)

target_include_directories(Shared
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
)
