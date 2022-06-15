#pragma once

#include <SDL_stdinc.h>
#include "bitsery/ext/value_range.h"
#include <array>

namespace AM
{
/**
 * Represents the states of a particular entity's inputs at a point in time.
 */
struct Input {
public:
    enum Type : Uint8 { XUp, XDown, YUp, YDown, ZUp, ZDown, NumTypes, None };

    enum State : Uint8 { Released, Pressed };

    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** Holds the current state of the inputs, indexed by Input::Type. */
    typedef std::array<State, Type::NumTypes> StateArr;
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
