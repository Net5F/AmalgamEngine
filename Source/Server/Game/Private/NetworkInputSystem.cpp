#include "NetworkInputSystem.h"
#include "World.h"
#include "NetworkServer.h"
#include "Peer.h"
#include "SharedDefs.h"
#include <memory>
#include <iostream>

AM::NetworkInputSystem::NetworkInputSystem(World& inWorld, NetworkServer& inNetwork)
: world(inWorld), network(inNetwork)
{
}

void AM::NetworkInputSystem::processInputEvents()
{
    // Process input messages for all clients.
    for (std::shared_ptr<Peer> client : network.getClients()) {
        BinaryBufferPtr responseBuffer = network.receive(client);
        if (responseBuffer != nullptr) {
            // Decode the message.
            const fb::Message* message = fb::GetMessage(responseBuffer->data());
            if (message->content_type() != fb::MessageContent::EntityUpdate) {
                std::cerr << "Expected EntityUpdate but got something else." << std::endl;
                continue;
            }
            auto entityUpdate = static_cast<const fb::EntityUpdate*>(message->content());

            // Pull out the client's single entity.
            auto clientEntity = entityUpdate->entities()->Get(0);
            EntityID clientEntityID = clientEntity->id();

            // Update the server entity's InputComponent.
            std::array<Input::State, Input::NumTypes>& entityInputStates =
                world.inputs[clientEntityID].inputStates;
            auto clientInputStates = clientEntity->inputComponent()->inputStates();
            for (unsigned int i = 0; i < Input::NumTypes; ++i) {
                entityInputStates[i] = convertToAMInputState(clientInputStates->Get(i));

                std::cout << "InputState: {" << i << ", " << entityInputStates[i] << "}"
                << std::endl;
            }

            // Flag the entity as dirty.
            world.entityIsDirty[clientEntityID] = true;
        }
    }
}

AM::Input::State AM::NetworkInputSystem::convertToAMInputState(fb::InputState state)
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
