#pragma once

#include "GameDefs.h"
#include <SDL_stdinc.h>
#include "bitsery/ext/value_range.h"

namespace AM
{

/**
 * Represents the states of a particular entity's inputs at a point in time.
 */
struct Input {
public:
    enum Type : Uint8 {
        None,
        Up,
        Left,
        Down,
        Right,
        NumTypes
    };

    enum State : Uint8 { Released, Pressed };

    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** Holds the current state of the inputs, indexed by Input::Type. */
    typedef std::array<State, Type::NumTypes> StateArr;
    StateArr inputStates{};

    //--------------------------------------------------------------------------
    // Non-replicated data
    //--------------------------------------------------------------------------
    /**
     * Indicates whether we've received new input data for the entity since the
     * last time it was processed.
     *
     * Since input gets processed before movement, we assume that this also
     * shows which entities need to be broadcast after moving.
     */
    bool isDirty{false};
};

template<typename S>
void serialize(S& serializer, Input& input)
{
    // Bit pack the input array.
    // It's an array of 2-value enums, so we can make it pretty small.
    serializer.enableBitPacking(
        [&input](typename S::BPEnabledType& sbp) {
            sbp.container(
                input.inputStates,
                [](typename S::BPEnabledType& sbp, Input::State& inputState) {
                    constexpr bitsery::ext::ValueRange<Input::State> range{
                        Input::State::Released, Input::State::Pressed};
                    sbp.ext(inputState, range);
                });
        });
}

} // namespace AM
