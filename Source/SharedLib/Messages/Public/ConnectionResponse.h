#pragma once

#include "EngineMessageType.h"
#include "entt/entity/registry.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Contains a connection response, sent from the server to the client.
 */
struct ConnectionResponse {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::ConnectionResponse};

    /** The tick that the server is telling the client to assume. */
    Uint32 tickNum{0};

    /** The sim ID that the server has assigned to this client's player entity.
     */
    entt::entity entity{entt::null};

    /** The length, in chunks, of the tile map's X axis. */
    Uint16 mapXLengthChunks{0};

    /** The length, in chunks, of the tile map's Y axis. */
    Uint16 mapYLengthChunks{0};

    /** The length, in chunks, of the tile map's Z axis. */
    Uint16 mapZLengthChunks{0};
};

template<typename S>
void serialize(S& serializer, ConnectionResponse& connectionResponse)
{
    serializer.value4b(connectionResponse.tickNum);
    serializer.value4b(connectionResponse.entity);
    serializer.value2b(connectionResponse.mapXLengthChunks);
    serializer.value2b(connectionResponse.mapYLengthChunks);
    serializer.value2b(connectionResponse.mapZLengthChunks);
}

} // End namespace AM
