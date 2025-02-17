#pragma once

#include "AudioVisualEffectID.h"
#include "AudioVisualEffectInstance.h"
#include "ItemData.h"
#include "TileMap.h"
#include "EntityLocator.h"
#include "CollisionLocator.h"
#include "IDPool.h"
#include "entt/entity/registry.hpp"

struct SDL_Rect;

namespace AM
{
namespace Client
{
class GraphicData;

/**
 * Owns and manages the persistence of all world state.
 *
 * The client's world state consists of:
 *   Map data
 *     See TileMap.h
 *   Entity data
 *     Maintained at runtime in an ECS registry.
 *
 * Also provides helpers for common uses of world state.
 */
class World
{
public:
    World(GraphicData& graphicData);

    /** Entity data registry. */
    entt::registry registry;

    /** Our active audio/visual effect instances. */
    std::unordered_map<AudioVisualEffectID, AudioVisualEffectInstance>
        audioVisualEffects;

    /** Item data templates. */
    ItemData itemData;

    /** The entity that this client is controlling. */
    entt::entity playerEntity;

    /** Spatial partitioning grid for efficiently locating entities by
        position. */
    EntityLocator entityLocator;

    /** Spatial partitioning grid for efficiently locating entities and tile 
        layers by their collision volumes. */
    CollisionLocator collisionLocator;

    /** The tile map that makes up the world. */
    TileMap tileMap;

    /**
     * Adds a client-only AV effect instance to the world.
     *
     * @param selfEntity The entity that the effect is targeting. If avEffect
     *                   ever requires a Self entity, this is required. Else,
     *                   this may be entt::null.
     * @param targetEntity The entity that the effect is targeting. If avEffect
     *                     ever requires a Target entity, this is required. 
     *                     Else, this may be entt::null.
     * @return If successful, returns the new effect's ID. Else, returns 
     *         null (effect has no phases, required entity was invalid).
     */
    AudioVisualEffectID addAudioVisualEffect(const AudioVisualEffect& avEffect,
                                             entt::entity selfEntity,
                                             entt::entity targetEntity);

    /**
     * Removes an AV effect from the world.
     * 
     * Only needed if you want to prematurely cancel an effect, or remove an 
     * infinite-duration effect. Otherwise, all effects are automatically 
     * removed when their last phase completes.
     */
    void remAudioVisualEffect(AudioVisualEffectID avEffectID);

private:
    /** Used for generating runtime IDs for AV effect instances. */
    IDPool audioVisualEffectIDPool;
};

} // namespace Client
} // namespace AM
