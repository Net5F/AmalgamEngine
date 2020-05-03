#include "NetworkMovementSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageUtil.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

NetworkMovementSystem::NetworkMovementSystem(Game& inGame, World& inWorld,
                                             Network& inNetwork)
: game(inGame), world(inWorld), network(inNetwork)
{
}

void NetworkMovementSystem::processServerMovements()
{
    // Check for a message from the server.
    BinaryBufferPtr responseBuffer = network.receive();

    while (responseBuffer != nullptr) {
        // Check that we got an EntityUpdate and ready it for reading.
        const fb::Message* message = fb::GetMessage(responseBuffer->data());
        if (message->content_type() != fb::MessageContent::EntityUpdate) {
            std::cerr << "Expected EntityUpdate but got something else." << std::endl;
            return;
        }
        auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());

        // Pull out the vector of entities.
        auto entities = entityUpdate->entities();

        // Iterate through the entities, updating all local data.
        for (auto entityIt = entities->begin(); entityIt != entities->end(); ++entityIt) {
            EntityID entityID = (*entityIt)->id();

            // If the entity doesn't exist, add it to our list.
            if (!(world.entityExists(entityID))) {
                std::cout << "New entity added. ID: " << entityID << std::endl;
                world.addEntity((*entityIt)->name()->str(), entityID);

                // TODO: Get this info from the server.
                // Get the same texture as the player.
                world.sprites[entityID].texturePtr =
                    world.sprites[world.getPlayerID()].texturePtr;
                world.sprites[entityID].posInTexture =
                    world.sprites[world.getPlayerID()].posInTexture;
                world.sprites[entityID].width = 64;
                world.sprites[entityID].height = 64;

                world.attachComponent(entityID, ComponentFlag::Input);
                world.attachComponent(entityID, ComponentFlag::Movement);
                world.attachComponent(entityID, ComponentFlag::Position);
                world.attachComponent(entityID, ComponentFlag::Sprite);
            }

            // Only update movements and inputs if we weren't the cause of the inputs.
            if (entityID != world.getPlayerID()) {
                /* Update the inputs. */
                std::array<Input::State, Input::NumTypes>& entityInputStates =
                    world.inputs[entityID].inputStates;
                auto clientInputStates = (*entityIt)->inputComponent()->inputStates();
                for (unsigned int i = 0; i < Input::NumTypes; ++i) {
                    entityInputStates[i] = MessageUtil::convertToAMInputState(
                        clientInputStates->Get(i));
                }

                /* Update the movements. */
                MovementComponent& movement = world.movements[entityID];
                auto newMovement = (*entityIt)->movementComponent();
                movement.velX = newMovement->velX();
                movement.velY = newMovement->velY();
                movement.maxVelX = newMovement->maxVelX();
                movement.maxVelY = newMovement->maxVelY();
            }

            /* Update the positions. */
            PositionComponent& position = world.positions[entityID];
            auto newPosition = (*entityIt)->positionComponent();

            DebugInfo("%d: (%f, %f) -> (%f, %f)", entityID, position.x, position.y,
                newPosition->x(), newPosition->y());
            position.x = newPosition->x();
            position.y = newPosition->y();
        }

        responseBuffer = network.receive();
    } // End while
}

} // namespace Client
} // namespace AM
