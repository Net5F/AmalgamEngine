#include "ReplicatedMovementSystem.h"
#include "SimulationContext.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "MovementHelpers.h"
#include "MovementUpdate.h"
#include "Name.h"
#include "EnttGroups.h"
#include "Transforms.h"
#include "Config.h"
#include "SharedConfig.h"
#include "Log.h"
#include "entt/entity/registry.hpp"
#include <memory>

namespace AM
{
namespace Client
{
ReplicatedMovementSystem::ReplicatedMovementSystem(
    const SimulationContext& inSimContext)
: simulation{inSimContext.simulation}
, world{inSimContext.simulation.getWorld()}
, network{inSimContext.network}
, entityMover{world.registry, world.tileMap, world.entityLocator,
              world.collisionLocator}
, movementUpdateQueue{inSimContext.networkEventDispatcher}
, lastProcessedTick{0}
{
}

void ReplicatedMovementSystem::updateReplicatedEntities()
{
    if (Config::RUN_OFFLINE) {
        // No need to process replicated entities if we're running offline.
        return;
    }

    // If this is our first run, initialize our lastProcessedTick.
    if (lastProcessedTick == 0) {
        initLastProcessedTick();
    }

    // While we haven't reached the desired tick and have more data to process.
    Uint32 desiredTick{simulation.getReplicationTick()};
    Uint32 lastReceivedTick{network.worldEndpoint.getLastReceivedTick()};
    while ((lastProcessedTick < desiredTick)
           && (lastProcessedTick < lastReceivedTick)) {
        // Move all replicated entities as if their inputs didn't change.
        moveAllReplicatedEntities();

        // If there's an update message waiting.
        std::shared_ptr<const MovementUpdate>* movementUpdate{
            movementUpdateQueue.peek()};
        if ((movementUpdate != nullptr)) {
            Uint32 messageTick{(*movementUpdate)->tickNum};
            AM_ASSERT((messageTick >= (lastProcessedTick + 1)),
                      "Processed replicated entity movement out of order.");

            // If the update is for this tick, apply it.
            if (messageTick == (lastProcessedTick + 1)) {
                applyUpdateMessage(*(movementUpdate->get()));
                movementUpdateQueue.pop();
            }
        }

        lastProcessedTick++;
    }

    // Signal the updated components to any observers.
    emitUpdateSignals();

    // If we wanted to process more ticks but ran out of data, log it.
    if (lastProcessedTick < desiredTick) {
        LOG_WARN("Ran out of replicated movement data. lastProcessed: %u, "
                 "desired: %u, "
                 "lastReceived: %u, queueSize: %u",
                 lastProcessedTick, desiredTick, lastReceivedTick,
                 movementUpdateQueue.size());
    }
}

void ReplicatedMovementSystem::initLastProcessedTick()
{
    // Since we aren't ran until a server connection is established, the sim's
    // currentTick is set to what the server gave us in ConnectionResponse.
    // Our first received updates will be for that tick, so we init to
    // currentTick - 1 to prepare for processing them.
    lastProcessedTick = simulation.getReplicationTick() - 1;
}

void ReplicatedMovementSystem::moveAllReplicatedEntities()
{
    auto movementGroup{EnttGroups::getMovementGroup(world.registry)};
    for (auto [entity, input, position, previousPosition, movement,
               movementMods, rotation, collision, collisionBitSets,
               graphicState] : movementGroup.each()) {
        // Save their old position.
        previousPosition = position;

        // Move the entity.
        entityMover.moveEntity(
            {.entity{entity},
             .inputStates{input.inputStates},
             .position{position},
             .previousPosition{previousPosition},
             .movement{movement},
             .movementMods{movementMods},
             .rotation{rotation},
             .collision{collision},
             .collisionBitSets{collisionBitSets},
             .deltaSeconds{SharedConfig::SIM_TICK_TIMESTEP_S}});
    }
}

void ReplicatedMovementSystem::applyUpdateMessage(
    const MovementUpdate& movementUpdate)
{
    entt::registry& registry{world.registry};
    auto movementGroup{EnttGroups::getMovementGroup(registry)};

    // Apply each updated entity's new state.
    for (const MovementState& movementState : movementUpdate.movementStates) {
        // Check that the entity exists.
        // TODO: There's possibly an issue here if we receive a MovementUpdate
        //       while the sim happens to be at this system, and the update's
        //       tick is up for processing. We might end up here before
        //       EntityLifetimeSystem was able to construct the entity.
        entt::entity entity{movementState.entity};
        if (!(registry.valid(entity))) {
            LOG_FATAL(
                "Received update for invalid entity: %u. Message tick: %u",
                entity, movementUpdate.tickNum);
        }

        // Get the entity's components.
        auto [input, position, previousPosition, movement, rotation, collision]
            = movementGroup.get<Input, Position, PreviousPosition, Movement,
                                Rotation, Collision>(entity);

        // Apply the received component updates.
        input = movementState.input;
        position = movementState.position;
        movement = movementState.movement;
        rotation = MovementHelpers::calcRotation(rotation, input.inputStates);

        // If the previous position hasn't been initialized, set it to the
        // current position so they don't lerp in from the origin.
        if (!(previousPosition.isInitialized)) {
            previousPosition = position;
            previousPosition.isInitialized = true;
        }

        // Move their collision box to their new position.
        collision.worldBounds
            = Transforms::modelToWorldEntity(collision.modelBounds, position);
    }
}

void ReplicatedMovementSystem::emitUpdateSignals()
{
    // Emit update signals to any observers.
    auto view{world.registry.view<Position, PreviousPosition>()};
    for (entt::entity entity : view) {
        world.registry.patch<Position>(entity, [](auto&) {});
    }
}

} // namespace Client
} // namespace AM
