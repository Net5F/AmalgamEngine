#pragma once

namespace AM
{
/**
 * A tag struct used to tell if an entity belongs to a client or not.
 *
 * Terms:
 *   "Client entities" are controlled by a client.
 *   "Non-client entities" are controlled by the server (objects, npcs, etc).
 *   "Player entity" is the client entity controlled by this client.
 *   "Non-player-controlled entity (NPC)" is any non-player entity. This may be
 *   a client entity or a non-client entity.
 */
struct IsClientEntity {
    // Note: No networked data.
};

template<typename S>
void serialize(S& serializer, IsClientEntity& isClientEntity)
{
    // Note: No data to serialize.
}

} // namespace AM
