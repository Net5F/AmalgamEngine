#pragma once

#include "SDL_stdinc.h"

namespace AM
{

/**
 * This struct represents a connection response from the server to the client.
 */
struct ConnectionResponse
{
    /** The sim ID that the server has assigned to this client's player entity. */
    Uint32 entityID = 0;

    /** Position (spawn point or last logout). */
    float x = 0;
    float y = 0;
};

template <typename S>
void serialize(S& serializer, ConnectionResponse connectionResponse)
{
    serializer.value4b(connectionResponse.entityID);
    serializer.value4b(connectionResponse.x);
    serializer.value4b(connectionResponse.y);
}

} // End namespace AM
