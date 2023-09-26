#pragma once

#include <SDL_stdinc.h>
#include "bitsery/ext/value_range.h"
#include <array>

namespace AM
{
/**
 * Represents a particular entity's current input states.
 *
 * In the server, when this component is updated, MovementSyncSystem auto-sends
 * movement state updates to all nearby clients.
 * If you want to sync an entity's movement state (e.g. Position) without 
 * changing its inputs, you can just registry.patch() with no changes.
 */
struct Input {
    enum Type : Uint8 { XUp, XDown, YUp, YDown, ZUp, ZDown, Count, None };

    enum State : Uint8 { Released, Pressed };

    /** Holds the current state of the inputs, indexed by Input::Type. */
    using StateArr = std::array<State, Type::Count>;
    StateArr inputStates{};
};

template<typename S>
void serialize(S& serializer, Input& input)
{
    // Bit pack the input array.
    // It's an array of 2-value enums, so we can make it pretty small.
    // Note: We expect the outer context (such as EntityUpdate) to
    //       enable bit packing.
    serializer.container(input.inputStates, [](typename S::BPEnabledType& sbp,
                                               Input::State& inputState) {
        constexpr bitsery::ext::ValueRange<Input::State> range{
            Input::State::Released, Input::State::Pressed};
        sbp.ext(inputState, range);
    });

    // Align after bit-packing to make sure the following bytes can be easily
    // processed.
    serializer.adapter().align();
}

} // namespace AM
