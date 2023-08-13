#pragma once

namespace AM
{
/**
 * The types of entity that the engine supports.
 * We use this concept to give the player a simple way to think about entities.
 * This aims to avoid overwhelming them with abstract classifications based on 
 * what components an entity has.
 *
 * Note: From the Client's point of view, all peers and AI are sometimes 
 *       considered "NPCs". EntityType::NPC however means AI specifically.
 * Note: An alternative approach would have been to use tag components
 *       (IsClientEntity, etc). We purposely chose not to do this, because 
 *       it might lead to degenerate behavior (e.g. systems acting on "all 
 *       NPCs", instead of "all entities with an AIScript component").
 */
enum class EntityType
{
    /* Controlled by a client. From the Client's perspective, this may be either
       the "player entity" (the entity controlled by this client's player), or 
       a peer's player entity. */
    ClientEntity,
    /* Doesn't move. Can be interacted with. */
    DynamicObject,
    /* Can move. Has an AI script. Can be interacted with. */
    NPC,
    /* A volume in the world. Calls a script when an entity enters or leaves 
       it. */
    TriggerVolume
};

} // namespace AM
