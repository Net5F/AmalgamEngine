#include "Debug.h"
#include "Message_generated.h"

using namespace AM;

int main()
{
    // Prepare the inputs.
    static const unsigned int numInputs = 6;
    fb::InputState fbInputStates[numInputs];
    for (uint8_t i = 0; i < numInputs; ++i) {
        // Translate the Input::State enum to fb::InputState.
        fbInputStates[i] = fb::InputState::Pressed;
    }

    flatbuffers::FlatBufferBuilder builder(512);
    flatbuffers::Offset<flatbuffers::Vector<fb::InputState>> inputVector =
        builder.CreateVector(fbInputStates, numInputs);

    // Build the InputComponent.
    flatbuffers::Offset<fb::InputComponent> inputComponent = fb::CreateInputComponent(
        builder, inputVector);

    // Build the PositionComponent.
    flatbuffers::Offset<fb::PositionComponent> positionComponent =
        fb::CreatePositionComponent(builder, 100, 100);

    // Build the MovementComponent.
    flatbuffers::Offset<fb::MovementComponent> movementComponent =
        fb::CreateMovementComponent(builder, 100, 100, 100, 100);

    // Build the Entity.
    auto entityName = builder.CreateString("Player1");
    fb::EntityBuilder entityBuilder(builder);
    entityBuilder.add_id(1);
    entityBuilder.add_name(entityName);

    // Mark the components that we're sending.
    entityBuilder.add_flags(0);
    entityBuilder.add_positionComponent(positionComponent);
    entityBuilder.add_movementComponent(movementComponent);
    entityBuilder.add_inputComponent(inputComponent);

    // Wrap the Entity in an EntityUpdate.
    std::vector<flatbuffers::Offset<fb::Entity>> entityVector;
    entityVector.push_back(entityBuilder.Finish());
    auto serializedEntities = builder.CreateVector(entityVector);
    flatbuffers::Offset<fb::EntityUpdate> entityUpdate =
        fb::CreateEntityUpdate(builder, serializedEntities);

    // Wrap the EntityUpdate in a Message.
    fb::MessageBuilder messageBuilder(builder);
    messageBuilder.add_tickTimestamp(40000);
    messageBuilder.add_content_type(fb::MessageContent::EntityUpdate);
    messageBuilder.add_content(entityUpdate.Union());
    flatbuffers::Offset<fb::Message> message = messageBuilder.Finish();

    // Get the size.
    builder.Finish(message);
    DebugInfo("Message size: %u", builder.GetSize());

    return 0;
}
