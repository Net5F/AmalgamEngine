#include "SpriteUpdateSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "SpriteData.h"
#include "EntityType.h"
#include "Collision.h"
#include "SpriteSets.h"
#include "Sprite.h"
#include "Transforms.h"
#include "Log.h"

namespace AM
{
namespace Client
{
SpriteUpdateSystem::SpriteUpdateSystem(Simulation& inSimulation, World& inWorld,
                                       Network& inNetwork,
                                       SpriteData& inSpriteData)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, spriteData{inSpriteData}
, spriteChangeQueue{network.getEventDispatcher()}
{
}

void SpriteUpdateSystem::updateSprites()
{
    entt::registry& registry{world.registry};

    // Note: We process sprite updates based on the replication tick instead of 
    //       the current tick. This means that, if the server observes our 
    //       player entity colliding with the new sprite, it'll take longer 
    //       before we replicate it (and rubberband the player).
    //       This approach was chosen because there's always going to be some 
    //       rubberbanding for the player in that scenario, and this approach 
    //       lets us keep NPC replication perfect.
    Uint32 desiredTick{simulation.getReplicationTick()};

    // Process any received sprite updates, stopping at the desired tick.
    SpriteChange* spriteChange{spriteChangeQueue.peek()};
    while ((spriteChange != nullptr)
           && (spriteChange->tickNum <= desiredTick)) {
        entt::entity updatedEntity{spriteChange->entity};

        auto [entityType, position, sprite, collision]
            = registry.get<EntityType, Position, Sprite, Collision>(
                updatedEntity);
        if ((entityType == EntityType::ClientEntity)
            || (entityType == EntityType::NPC)) {
            //// Client entity and NPC Rotation components control which sprite
            //// index they use, so we ignore the given index.
            // TODO: Switch this to use character sprite sets.
            // TEMP
            const Sprite& newSprite{
                spriteData.getSprite(spriteChange->spriteNumericID)};
            sprite = newSprite;
            collision.modelBounds = newSprite.modelBounds;
            collision.worldBounds = Transforms::modelToWorldCentered(
                newSprite.modelBounds, position);
            // TEMP
        }
        else if (entityType == EntityType::DynamicObject) {
            // Dynamic objects don't move, so we change their rotation to match 
            // the sprite index they're set to.
            auto [spriteSet, rotation]
                = registry.get<ObjectSpriteSet, Rotation>(updatedEntity);

            const ObjectSpriteSet& newSpriteSet{
                spriteData.getObjectSpriteSet(spriteChange->spriteSetID)};
            spriteSet = newSpriteSet;
            const Sprite& newSprite{
                *(newSpriteSet.sprites[spriteChange->spriteIndex])};
            sprite = newSprite;
            rotation.direction
                = static_cast<Rotation::Direction>(spriteChange->spriteIndex);
            collision.modelBounds = newSprite.modelBounds;
            collision.worldBounds = Transforms::modelToWorldCentered(
                newSprite.modelBounds, position);
        }

        spriteChangeQueue.pop();
        spriteChange = spriteChangeQueue.peek();
    }
}

} // namespace Client
} // namespace AM