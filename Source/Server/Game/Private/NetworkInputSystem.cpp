#include "NetworkInputSystem.h"
#include "World.h"
#include "Network.h"
#include "Peer.h"
#include "SharedDefs.h"
#include "MessageUtil.h"
#include "Debug.h"
#include <memory>

namespace AM
{
namespace Server
{

NetworkInputSystem::NetworkInputSystem(World& inWorld, Network& inNetwork)
: world(inWorld), network(inNetwork)
{
}

void NetworkInputSystem::processInputEvents()
{
    // Process all the input messages that existed when we first check.
    // (We don't want to get stuck in a loop of constantly receiving.)
    unsigned int waitingMessageCount = network.getNumInputMessagesWaiting();
    for (unsigned int i = 0; i < waitingMessageCount; ++i) {
        BinaryBufferPtr inputMessage = network.receiveInputMessage();
        if (inputMessage == nullptr) {
            DebugInfo(
                "Failed to receive input message after getting count (this shouldn't happen).")
        }

        // Decode the message.
        const fb::Message* message = fb::GetMessage(inputMessage->data());
        if (message->content_type() != fb::MessageContent::EntityUpdate) {
            DebugInfo("Expected EntityUpdate but got something else.");
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
            entityInputStates[i] = MessageUtil::convertToAMInputState(
                clientInputStates->Get(i));
        }

        // Flag the entity as dirty.
        world.entityIsDirty[clientEntityID] = true;
    }
}

} // namespace Server
} // namespace AM
