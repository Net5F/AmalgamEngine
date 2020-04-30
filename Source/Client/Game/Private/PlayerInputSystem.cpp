#include "PlayerInputSystem.h"
#include "Game.h"
#include "World.h"
#include "Network.h"
#include "MessageUtil.h"
#include "Debug.h"

namespace AM
{
namespace Client
{

PlayerInputSystem::PlayerInputSystem(Game& inGame, World& inWorld, Network& inNetwork)
: game(inGame),
  world(inWorld),
  network(inNetwork),
  builder(BUILDER_BUFFER_SIZE),
  stateIsDirty(false)
{
}

void PlayerInputSystem::processInputEvent(SDL_Event& event)
{
    // Process all events.
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        Input::State inputType =
        (event.type == SDL_KEYDOWN) ? Input::Pressed : Input::Released;
        Input keyInput = { Input::None, inputType };

        switch (event.key.keysym.sym)
        {
            case SDLK_w:
                keyInput.type = Input::Up;
                break;
            case SDLK_a:
                keyInput.type = Input::Left;
                break;
            case SDLK_s:
                keyInput.type = Input::Down;
                break;
            case SDLK_d:
                keyInput.type = Input::Right;
                break;
        }

        // Push the input to the player's InputComponent.
        EntityID player = world.getPlayerID();
        Input::State& entityState = world.inputs[player].inputStates[keyInput.type];

        // Track changes.
        if (entityState != keyInput.state) {
            stateIsDirty = true;
            entityState = keyInput.state;
        }
    }
}

void PlayerInputSystem::sendInputState()
{
    if (!stateIsDirty) {
        // If there's no change, don't send anything.
        return;
    }

    /* Send the updated state to the server. */
    // Prep the builder for a new message.
    builder.Clear();

    EntityID playerID = world.getPlayerID();

    // Create the vector of entity data.
    std::vector<flatbuffers::Offset<fb::Entity>> entityVector;
    entityVector.push_back(serializeEntity(playerID));
    auto serializedEntity = builder.CreateVector(entityVector);

    // Build an EntityUpdate.
    flatbuffers::Offset<fb::EntityUpdate> entityUpdate = fb::CreateEntityUpdate(builder, game.getCurrentTick()
    , serializedEntity);

    // Build a Message.
    fb::MessageBuilder messageBuilder(builder);
    messageBuilder.add_content_type(fb::MessageContent::EntityUpdate);
    messageBuilder.add_content(entityUpdate.Union());
    flatbuffers::Offset<fb::Message> message = messageBuilder.Finish();
    builder.Finish(message);

    // Send the message.
    Uint8* buffer = builder.GetBufferPointer();
    network.send(
        std::make_shared<std::vector<Uint8>>(buffer, (buffer + builder.GetSize())));

    stateIsDirty = false;
}

flatbuffers::Offset<AM::fb::Entity> PlayerInputSystem::serializeEntity(
EntityID playerID)
{
    // Translate the inputs to fb's enum.
    fb::InputState fbInputStates[Input::Type::NumTypes];
    std::array<Input::State, Input::NumTypes>& playerInputStates =
        world.inputs[playerID].inputStates;
    for (uint8_t i = 0; i < Input::Type::NumTypes; ++i) {
        // Translate the Input::State enum to fb::InputState.
        fbInputStates[i] = MessageUtil::convertToFbInputState(playerInputStates[i]);
    }
    flatbuffers::Offset<flatbuffers::Vector<fb::InputState>> inputVector =
        builder.CreateVector(fbInputStates, Input::Type::NumTypes);

    // Build the inputComponent.
    flatbuffers::Offset<fb::InputComponent> inputComponent = fb::CreateInputComponent(
        builder, inputVector);

    // Build the Entity.
    auto entityName = builder.CreateString(world.entityNames[playerID]);
    fb::EntityBuilder entityBuilder(builder);
    entityBuilder.add_id(playerID);
    entityBuilder.add_name(entityName);

    // Mark that we only are sending the InputComponent.
    entityBuilder.add_flags(ComponentFlag::Input);
    entityBuilder.add_inputComponent(inputComponent);

    return entityBuilder.Finish();
}

} // namespace Client
} // namespace AM
