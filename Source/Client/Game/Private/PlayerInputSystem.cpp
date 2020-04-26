#include <PlayerInputSystem.h>
#include "World.h"
#include "Network.h"

namespace AM
{
namespace Client
{

PlayerInputSystem::PlayerInputSystem(World& inWorld, Network& inNetwork)
: world(inWorld), network(inNetwork), builder(BUILDER_BUFFER_SIZE)
{
}

Input PlayerInputSystem::processInputEvents()
{
    // Process all events.
    SDL_Event event;
    bool stateChanged = false;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return {Input::Exit, Input::Pressed};
        }
        else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
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
                case SDLK_ESCAPE:
                    return {Input::Exit, Input::Pressed};
            }

            // Push the input to the player's InputComponent.
            EntityID player = world.getPlayerID();
            Input::State& entityState = world.inputs[player].inputStates[keyInput.type];

            // Only update on change.
            if (entityState != keyInput.state) {
                stateChanged = true;
                entityState = keyInput.state;
            }
        }
    }

    // If a change occurred, send the updated player input state to the server.
    if (stateChanged) {
        sendInputState();
    }

    return {Input::None, Input::Invalid};
}

void PlayerInputSystem::sendInputState()
{
    // Prep the builder for a new message.
    builder.Clear();

    EntityID playerID = world.getPlayerID();

    // Translate the inputs to fb's enum.
    fb::InputState fbInputStates[Input::Type::NumTypes];
    std::array<Input::State, Input::NumTypes>& playerInputStates =
        world.inputs[playerID].inputStates;
    for (uint8_t i = 0; i < Input::Type::NumTypes; ++i) {
        // Translate the Input::State enum to fb::InputState.
        fbInputStates[i] = convertToFbInputState(playerInputStates[i]);
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

    std::vector<flatbuffers::Offset<fb::Entity>> entityVector;
    entityVector.push_back(entityBuilder.Finish());
    auto entity = builder.CreateVector(entityVector);

    // Build an EntityUpdate.
    flatbuffers::Offset<fb::EntityUpdate> entityUpdate = fb::CreateEntityUpdate(builder,
        entity);

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
}

fb::InputState PlayerInputSystem::convertToFbInputState(Input::State state)
{
    switch (state)
    {
        case Input::Invalid:
            return fb::InputState::Invalid;
            break;
        case Input::Pressed:
            return fb::InputState::Pressed;
            break;
        case Input::Released:
            return fb::InputState::Released;
            break;
        default:
            return fb::InputState::Invalid;
    }
}

} // namespace Client
} // namespace AM
