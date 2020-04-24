#include "NetworkMovementSystem.h"
#include "World.h"
#include "NetworkClient.h"
#include "Message_generated.h"

AM::NetworkMovementSystem::NetworkMovementSystem(World& inWorld, NetworkClient& inNetwork)
: world(inWorld), network(inNetwork)
{
}

void AM::NetworkMovementSystem::processServerMovements()
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
            world.AddEntity((*entityIt)->name()->str(), entityID);

            // TODO: Get this info from the server.
            // Get the same texture as the player.
            world.sprites[entityID].texturePtr =
                world.sprites[world.getPlayerID()].texturePtr;
            world.sprites[entityID].posInTexture =
                world.sprites[world.getPlayerID()].posInTexture;
        }

        /* Update the positions. */
        PositionComponent& position = world.positions[entityID];
        auto newPosition = (*entityIt)->positionComponent();

        // TEMP: Print the updated position so we know something happened.
        if (entityID == world.getPlayerID()) {
            std::cout << "( " << position.x << ", " << position.y << ") -> ("
            << newPosition->x() << ", " << newPosition->y() << ")" << std::endl;
        }
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
