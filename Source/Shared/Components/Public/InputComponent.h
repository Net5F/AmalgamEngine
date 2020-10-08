#pragma once

#include "GameDefs.h"
#include "bitsery/ext/value_range.h"

namespace AM
{
struct InputComponent {
public:
    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** Holds the current state of the inputs, indexed by Input::Type. */
    InputStateArr inputStates = {};
};

template<typename S>
void serialize(S& serializer, InputComponent& inputComponent)
{
    // Bit pack the input array.
    // It's an array of 2-value enums, so we can make it pretty small.
    serializer.enableBitPacking(
        [&inputComponent](typename S::BPEnabledType& sbp) {
            sbp.container(
                inputComponent.inputStates,
                [](typename S::BPEnabledType& sbp, Input::State& inputState) {
                    constexpr bitsery::ext::ValueRange<Input::State> range{
                        Input::State::Released, Input::State::Pressed};
                    sbp.ext(inputState, range);
                });
        });
}

} // namespace AM
