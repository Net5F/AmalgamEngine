#include "Log.h"
//#include "Message_generated.h"
#include "SerializationTestResources/Basic_generated.h"

using namespace AM;

int main()
{
    // Set the builder large enough to hold 1000 entities.
    flatbuffers::FlatBufferBuilder builder(131072);

    // Prepare the inputs.
    const unsigned int numInputs = 6;
    fb::InputState fbInputStates[numInputs];
    for (uint8_t i = 0; i < numInputs; ++i) {
        // Translate the Input::State enum to fb::InputState.
        fbInputStates[i] = fb::InputState::Pressed;
    }
    flatbuffers::Offset<flatbuffers::Vector<fb::InputState>> inputVector
        = builder.CreateVector(fbInputStates, numInputs);

    // Build the entities.
    std::vector<flatbuffers::Offset<fb::Entity>> entityVector;

    const unsigned int numEntities = 1000;
    for (unsigned int i = 0; i < numEntities; ++i) {
        fb::EntityBuilder entityBuilder(builder);
        entityBuilder.add_id(1);
        entityBuilder.add_x(232.232);
        entityBuilder.add_y(592.2348);
        entityBuilder.add_z(49.2384);
        entityBuilder.add_velX(232.232);
        entityBuilder.add_velY(592.2348);
        entityBuilder.add_velZ(49.2384);
        entityBuilder.add_inputStates(inputVector);

        entityVector.push_back(entityBuilder.Finish());
    }

    // Wrap the Entities in an EntityUpdate.
    auto serializedEntities = builder.CreateVector(entityVector);
    flatbuffers::Offset<fb::EntityUpdate> entityUpdate
        = fb::CreateEntityUpdate(builder, serializedEntities);

    // Get the size.
    builder.Finish(entityUpdate);
    LOG_INFO("Message size: %u", builder.GetSize());
    return 0;
}
