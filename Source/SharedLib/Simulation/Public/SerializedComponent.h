#pragma once

#include "BinaryBuffer.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * The serialized and tagged data for a single component.
 */
struct SerializedComponent {
    /** Used as a "we should never hit this" cap on the buffer size. Only 
        checked in debug builds. */
    static constexpr std::size_t MAX_BUFFER_SIZE{1000};

    /** This component's type index within the relevant list:
          ReplicatedComponentTypes for init and update scenarios.
          PersistedComponentTypes for save and load scenarios.
        Note: The project owns the type lists and passes them to the engine.
              See ComponentTypeRegistry.h for more info. */
    Uint8 typeIndex{};

    /** Holds the serialized component data. */
    BinaryBuffer buffer{};
};

template<typename S>
void serialize(S& serializer, SerializedComponent& serializedComponent)
{
    serializer.value1b(serializedComponent.typeIndex);
    serializer.container1b(serializedComponent.buffer,
                           SerializedComponent::MAX_BUFFER_SIZE);
}

} // End namespace AM
