#include "Game.h"
#include "NetworkServer.h"

using namespace AM;

AM::Game::Game(NetworkServer& inNetwork)
: world()
, network(inNetwork)
, networkInputSystem(world, network)
, movementSystem(world, network)
, builder(BUILDER_BUFFER_SIZE)
, timeSinceTick(0)
{
    world.setSpawnPoint({64, 64});
}

void AM::Game::tick(double deltaMs)
{
    // Check if we should process this tick.
    timeSinceTick += deltaMs;
    if (timeSinceTick < TICK_INTERVAL_MS) {
        return;
    }
    else {
        timeSinceTick = 0;
    }

    // Add any new connections.
    std::vector<std::shared_ptr<Peer>> newClients =
        network.acceptNewClients();
    for (std::shared_ptr<Peer> client : newClients) {
        // Build their entity.
        EntityID newID = world.AddEntity("Player");
        const Position& spawnPoint = world.getSpawnPoint();
        world.positions[newID].x = spawnPoint.x;
        world.positions[newID].y = spawnPoint.y;
        world.movements[newID].maxVelX = 15;
        world.movements[newID].maxVelY = 15;
        world.AttachComponent(newID, ComponentFlag::Input);
        world.AttachComponent(newID, ComponentFlag::Movement);
        world.AttachComponent(newID, ComponentFlag::Position);
        world.AttachComponent(newID, ComponentFlag::Sprite);

        // Prep the builder for a new message.
        builder.Clear();

        // Send them their ID and spawn point.
        auto response = fb::CreateConnectionResponse(builder, newID, spawnPoint.x,
            spawnPoint.y);
        auto encodedMessage = fb::CreateMessage(builder,
            fb::MessageContent::ConnectionResponse, response.Union());
        builder.Finish(encodedMessage);

        Uint8* buffer = builder.GetBufferPointer();
        BinaryBufferSharedPtr message = std::make_shared<std::vector<Uint8>>(
        buffer, (buffer + builder.GetSize()));

        bool result = network.send(client, message);
        if (!result) {
            std::cerr << "Failed to send response." << std::endl;
        }
    }

    // Check for disconnects.
    network.checkForDisconnections();

    // Run all systems.
    networkInputSystem.processInputEvents();

    movementSystem.processMovements(33.3);
}
