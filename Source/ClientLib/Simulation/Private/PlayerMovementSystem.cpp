#include "PlayerMovementSystem.h"
#include "MovementHelpers.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "Position.h"
#include "Input.h"
#include "InputHistory.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "Rotation.h"
#include "Sprite.h"
#include "Collision.h"
#include "Transforms.h"
#include "SharedConfig.h"
#include "Config.h"
#include "Log.h"
#include "AMAssert.h"
#include <memory>

namespace AM
{
namespace Client
{
PlayerMovementSystem::PlayerMovementSystem(Simulation& inSimulation,
                                           World& inWorld, Network& inNetwork)
: simulation{inSimulation}
, world{inWorld}
, network{inNetwork}
, entityMover{world.registry, world.tileMap, world.entityLocator,
              world.collisionLocator}
, playerMovementUpdateQueue{inNetwork.getEventDispatcher()}
{
}

void PlayerMovementSystem::processMovement()
{
    // Save the old position.
    auto [position, previousPosition]
        = world.registry.get<Position, PreviousPosition>(world.playerEntity);
    previousPosition = position;

    // If we're online, process any updates from the server.
    if (!Config::RUN_OFFLINE) {
        // Apply any player entity updates from the server.
        // If an update is applied, the player entity's actual components will 
        // be moved back in time to match the update's state.
        Uint32 lastUpdateTick{processPlayerUpdates()};

        // If we received updates, replay the inputs that came after them.
        if (lastUpdateTick != 0) {
            replayInputs(lastUpdateTick);

            // Check if there was a mismatch between the position we had and
            // where the server thought we should be.
            if (previousPosition != position) {
                printMismatchInfo(lastUpdateTick);
            }
        }
    }

    // Process the player entity's movement for this tick.
    Input& input{world.registry.get<Input>(world.playerEntity)};
    movePlayerEntity(input.inputStates);

    // Signal the updated components to any observers.
    emitUpdateSignals();
}

Uint32 PlayerMovementSystem::processPlayerUpdates()
{
    entt::registry& registry{world.registry};
    auto [input, position, previousPosition, movement, rotation, collision,
          inputHistory]
        = registry.get<Input, Position, PreviousPosition, Movement, Rotation,
                       Collision, InputHistory>(world.playerEntity);

    /* Process any messages for us from the server. */
    PlayerMovementUpdate movementUpdate{};
    Uint32 lastUpdateTick{0};
    while (playerMovementUpdateQueue.pop(movementUpdate)) {
        // Check that the update's tick is in the past.
        Uint32 updateTick{movementUpdate.tickNum};
        Uint32 currentTick{simulation.getCurrentTick()};
        checkReceivedTickValidity(updateTick, currentTick);

        lastUpdateTick = updateTick;

        // Check that the diff is valid.
        Uint32 tickDiff{simulation.getCurrentTick() - updateTick};
        if (!checkTickDiffValidity(tickDiff)) {
            continue;
        }

        // Check if the received input disagrees with what we predicted.
        const Input& receivedInput{movementUpdate.input};
        if (receivedInput.inputStates != inputHistory.inputHistory[tickDiff]) {
            // Our prediction was wrong, accept the received input and set all
            // inputs in the history after the mismatched input to match it.
            // TODO: This may be incorrect, but it's uncommon and hard to
            // verify. We may want to more carefully overwrite existing inputs.
            input.inputStates = receivedInput.inputStates;
            for (std::size_t i{0}; i <= tickDiff; ++i) {
                inputHistory.inputHistory[i] = receivedInput.inputStates;
            }

            // Set our old position to the current so we aren't oddly lerping
            // back.
            previousPosition = position;
        }

        // Apply the rest of the received movement state.
        position = movementUpdate.position;
        movement = movementUpdate.movement;
        rotation = MovementHelpers::calcRotation(rotation, input.inputStates);
        collision.worldBounds
            = Transforms::modelToWorldEntity(collision.modelBounds, position);
    }

    return lastUpdateTick;
}

void PlayerMovementSystem::replayInputs(Uint32 lastUpdateTick)
{
    // Replay all inputs newer than lastUpdateTick, except the current
    // tick's input.
    Uint32 currentTick{simulation.getCurrentTick()};
    auto& inputHistory{world.registry.get<InputHistory>(world.playerEntity)};
    for (Uint32 tickToProcess{lastUpdateTick + 1}; tickToProcess < currentTick;
         ++tickToProcess) {
        // Check that the diff is valid.
        Uint32 tickDiff{currentTick - tickToProcess};
        checkTickDiffValidity(tickDiff);

        // Replay the input state and move the entity.
        movePlayerEntity(inputHistory.inputHistory[tickDiff]);
    }
}

void PlayerMovementSystem::movePlayerEntity(const Input::StateArr& inputStates)
{
    auto [position, previousPosition, movement, movementMods, rotation,
          collision, collisionBitSets]
        = world.registry
              .get<Position, PreviousPosition, Movement, MovementModifiers,
                   Rotation, Collision, CollisionBitSets>(world.playerEntity);

    // Move the entity.
    entityMover.moveEntity({.entity{world.playerEntity},
                            .inputStates{inputStates},
                            .position{position},
                            .previousPosition{previousPosition},
                            .movement{movement},
                            .movementMods{movementMods},
                            .rotation{rotation},
                            .collision{collision},
                            .collisionBitSets{collisionBitSets},
                            .deltaSeconds{SharedConfig::SIM_TICK_TIMESTEP_S}});
}

void PlayerMovementSystem::emitUpdateSignals()
{
    // Emit update signals to any observers.
    world.registry.patch<Position>(world.playerEntity, [](auto&) {});
}

void PlayerMovementSystem::printMismatchInfo(Uint32 lastUpdateTick)
{
    auto [position, previousPosition]
        = world.registry.get<Position, PreviousPosition>(world.playerEntity);

    LOG_INFO("Predicted position mismatched after replay: (%.6f, "
             "%.6f, %.6f) -> (%.6f, %.6f, %.6f)",
             previousPosition.x, previousPosition.y,
             previousPosition.z, position.x, position.y, position.z);
    LOG_INFO("lastUpdateTick: %u", lastUpdateTick);
}

void PlayerMovementSystem::checkReceivedTickValidity(
    [[maybe_unused]] Uint32 updateTick, [[maybe_unused]] Uint32 currentTick)
{
    AM_ASSERT((updateTick <= currentTick),
              "Received data for tick %u on tick %u. Server is in the future, "
              "can't replay inputs.",
              updateTick, currentTick);
}

bool PlayerMovementSystem::checkTickDiffValidity(Uint32 tickDiff)
{
    // The history includes the current tick, so we only have LENGTH - 1
    // worth of previous data to use (i.e. it's 0-indexed).
    if (tickDiff > (InputHistory::LENGTH - 1)) {
        LOG_ERROR("Too few items in the player input history. "
                  "Increase the length or reduce lag. tickDiff: %u, "
                  "historyLength: %u",
                  tickDiff, InputHistory::LENGTH);
        return false;
    }

    return true;
}

} // namespace Client
} // namespace AM
