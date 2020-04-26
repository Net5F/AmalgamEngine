#include "NetworkMovementSystem.h"
#include "World.h"
#include "Network.h"

namespace AM
{
namespace Client
{

NetworkMovementSystem::NetworkMovementSystem(World& inWorld, Network& inNetwork)
: world(inWorld), network(inNetwork)
{
}

void NetworkMovementSystem::processServerMovements()
{
    // Check for a message from the server.
    BinaryBufferPtr responseBuffer = network.receive();
    if (responseBuffer == nullptr) {
        // No message to process.
        return;
    }

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
            world.AddEntity((*entityIt)->name()->str(), entityID);

            // TODO: Get this info from the server.
            // Get the same texture as the player.
            world.sprites[entityID].texturePtr =
                world.sprites[world.getPlayerID()].texturePtr;
            world.sprites[entityID].posInTexture =
                world.sprites[world.getPlayerID()].posInTexture;
            world.sprites[entityID].posInWorld.h = 64;
            world.sprites[entityID].posInWorld.w = 64;

            world.AttachComponent(entityID, ComponentFlag::Input);
            world.AttachComponent(entityID, ComponentFlag::Movement);
            world.AttachComponent(entityID, ComponentFlag::Position);
            world.AttachComponent(entityID, ComponentFlag::Sprite);
        }

        /* Update the inputs. */
        std::array<Input::State, Input::NumTypes>& entityInputStates =
            world.inputs[entityID].inputStates;
        auto clientInputStates = (*entityIt)->inputComponent()->inputStates();
        for (unsigned int i = 0; i < Input::NumTypes; ++i) {
            entityInputStates[i] = convertToAMInputState(clientInputStates->Get(i));
        }

        /* Update the positions. */
        PositionComponent& position = world.positions[entityID];
        auto newPosition = (*entityIt)->positionComponent();

        // TEMP: Print the updated position so we know something happened.
        std::cout << entityID <<  ": ( " << position.x << ", " << position.y << ") -> ("
        << newPosition->x() << ", " << newPosition->y() << ")" << std::endl;
        position.x = newPosition->x();
        position.y = newPosition->y();

        /* Update the movements. */
        MovementComponent& movement = world.movements[entityID];
        auto newMovement = (*entityIt)->movementComponent();
        movement.velX = newMovement->velX();
        movement.velY = newMovement->velY();
        movement.maxVelX = newMovement->maxVelX();
        movement.maxVelY = newMovement->maxVelY();

        /* Move the sprites to the new positions. */
        SpriteComponent& sprite = world.sprites[entityID];
        sprite.posInWorld.x = world.positions[entityID].x;
        sprite.posInWorld.y = world.positions[entityID].y;
    }
}

Input::State NetworkMovementSystem::convertToAMInputState(fb::InputState state)
{
    switch (state)
    {
        case fb::InputState::Invalid:
            return Input::Invalid;
            break;
        case fb::InputState::Pressed:
            return Input::Pressed;
            break;
        case fb::InputState::Released:
            return Input::Released;
            break;
        default:
            return Input::State::Invalid;
    }
}

} // namespace Client
} // namespace AM
