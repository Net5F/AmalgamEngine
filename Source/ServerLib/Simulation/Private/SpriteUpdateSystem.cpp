#include "SpriteUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "EntityType.h"
#include "Position.h"
#include "Rotation.h"
#include "Sprite.h"
#include "Collision.h"
#include "SpriteSets.h"
#include "SpriteChange.h"
#include "ClientSimData.h"
#include "Cylinder.h"
#include "AnimationStateNeedsSync.h"
#include "Transforms.h"
#include "Log.h"

namespace AM
{
namespace Server
{
SpriteUpdateSystem::SpriteUpdateSystem(Simulation& inSimulation, World& inWorld,
                                       Network& inNetwork,
                                       SpriteData& inSpriteData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, spriteChangeQueue{inNetwork.getEventDispatcher()}
{
}

void SpriteUpdateSystem::updateSprites()
{
    entt::registry& registry{world.registry};

    // Process any waiting change requests.
    SpriteChange spriteChange{};
    while (spriteChangeQueue.pop(spriteChange)) {
        // If the given entity doesn't exist, skip this request.
        entt::entity updatedEntity{spriteChange.entity};
        if (!(world.entityIDIsInUse(updatedEntity))) {
            continue;
        }

        auto [entityType, position, collision, animationState]
            = registry.get<EntityType, Position, Collision, AnimationState>(
                updatedEntity);
        if ((entityType == EntityType::ClientEntity) ||
            (entityType == EntityType::NPC)) {
            // Client entity and NPC Rotation components control which sprite
            // index they use, so we ignore the given index.
            // TODO: When character sprite sets are added, update this.
            const ObjectSpriteSet& newSpriteSet{
                spriteData.getObjectSpriteSet(spriteChange.spriteSetID)};
            animationState.spriteSetID = newSpriteSet.numericID;
            animationState.spriteIndex = spriteChange.spriteIndex;

            const Sprite& newSprite{
                *(newSpriteSet.sprites[spriteChange.spriteIndex])};
            collision.modelBounds = newSprite.modelBounds;
            collision.worldBounds = Transforms::modelToWorldCentered(
                newSprite.modelBounds, position);
        }
        else if (entityType == EntityType::DynamicObject) {
            const ObjectSpriteSet& newSpriteSet{
                spriteData.getObjectSpriteSet(spriteChange.spriteSetID)};
            animationState.spriteSetID = newSpriteSet.numericID;
            animationState.spriteIndex = spriteChange.spriteIndex;

            const Sprite& newSprite{
                *(newSpriteSet.sprites[spriteChange.spriteIndex])};
            collision.modelBounds = newSprite.modelBounds;
            collision.worldBounds = Transforms::modelToWorldCentered(
                newSprite.modelBounds, position);
        }

        registry.emplace<AnimationStateNeedsSync>(updatedEntity);
    }
}

void SpriteUpdateSystem::sendSpriteUpdates()
{
    entt::registry& registry{world.registry};

    // Note: We make an assumption that every entity with a sprite also has 
    //       a position and entity type. This should always be true.
    auto view{
        registry.view<EntityType, AnimationState, AnimationStateNeedsSync>()};
    for (auto [updatedEntity, entityType, animationState] : view.each()) {
        // Get the list of entities that are in range of the updated entity 
        // and fill the update message struct.
        const std::vector<entt::entity>* entitiesInRange{nullptr};
        SpriteChange spriteChange{simulation.getCurrentTick(), updatedEntity};
        if (entityType == EntityType::ClientEntity) {
            // Clients already have their AOI list built.
            const ClientSimData& client{
                registry.get<ClientSimData>(updatedEntity)};
            entitiesInRange = &(client.entitiesInAOI);

            spriteChange.spriteSetID = animationState.spriteSetID;
            spriteChange.spriteIndex = animationState.spriteIndex;
        }
        else if ((entityType == EntityType::NPC) || 
                 (entityType == EntityType::DynamicObject)) {
            const Position& position{registry.get<Position>(updatedEntity)};
            entitiesInRange = &(world.entityLocator.getEntities(
                {position, SharedConfig::AOI_RADIUS}));

            // Note: NPCs and DynamicObjects have different sprite set types, 
            //       but it doesn't matter here.
            spriteChange.spriteSetID = animationState.spriteSetID;
            spriteChange.spriteIndex = animationState.spriteIndex;
        }
        else {
            LOG_FATAL("Unsupported entity type.");
        }

        // Build the update message.
        BinaryBufferSharedPtr message{network.serialize(spriteChange)};

        // Send the update to all nearby clients.
        for (entt::entity entity : *entitiesInRange) {
            if (registry.all_of<ClientSimData>(entity)) {
                const ClientSimData& client{
                    registry.get<ClientSimData>(entity)};
                network.send(client.netID, message);
            }
        }
    }

    registry.clear<AnimationStateNeedsSync>();
}

} // namespace Server
} // namespace AM
