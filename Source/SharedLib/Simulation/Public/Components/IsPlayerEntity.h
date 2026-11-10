#pragma once

namespace AM
{
/**
 * A tag struct used to tell if an entity is a player entity.
 *
 * Terms:
 *   "Player entities" are controlled by a client.
 *   "Non-player entities" are controlled by the server (objects, NPCs, etc).
 *
 * Client-only terms:
 *   "Local player" is the player entity controlled by this client.
 *   "Replicated entities" are all entities that this client doesn't predict,
 *   i.e. everything except the local player (other player entities and all
 *   non-player entities).
 */
struct IsPlayerEntity {
    // Note: No networked data.
};

template<typename S>
void serialize(S&, IsPlayerEntity&)
{
    // Note: No data to serialize.
}

} // namespace AM
