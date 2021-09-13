#pragma once

#include <SDL2/SDL_stdinc.h>
#include "entt/entity/registry.hpp"

namespace AM
{
/**
 * Contains a connection response, sent from the server to the client.
 */
struct ConnectionResponse {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr MessageType MESSAGE_TYPE = MessageType::ConnectionResponse;

    /** The tick that the server is telling the client to assume. */
    Uint32 tickNum{0};

    /** The sim ID that the server has assigned to this client's player entity.
     */
    entt::entity entity{entt::null};

    /** Position (spawn point or last logout). */
    float x{0};
    float y{0};
};

template<typename S>
void serialize(S& serializer, ConnectionResponse& connectionResponse)
{
    serializer.value4b(connectionResponse.tickNum);
    serializer.value4b(connectionResponse.entity);
    serializer.value4b(connectionResponse.x);
    serializer.value4b(connectionResponse.y);
}

} // End namespace AM
