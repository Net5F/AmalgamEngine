#pragma once

namespace AM
{
namespace Client
{
/**
 * A tag struct used to tell if an entity belongs to a client or not.
 * 
 * This is added to the player entity, and to any NPCs.
 * This is not added to non-client entities.
 *
 * Terms:
 *   "Client entities" are controlled by a client.
 *   "Non-client entities" are controlled by the server (objects, npcs, etc).
 *   "Player entity" is the client entity controlled by this client.
 *   "Non-player-controlled entity (NPC)" is any non-player entity. This may be 
 *   a client entity or a non-client entity.
 */
struct IsClientEntity {
};

} // namespace Client
} // namespace AM
