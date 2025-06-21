#pragma once

#include "bitsery/ext/std_bitset.h"
#include <bitset>

namespace AM
{
/**
 * Represents an entity's current movement-related input states.
 *
 * In the server, when this component is updated, MovementSyncSystem auto-sends
 * movement state updates to all nearby clients.
 * If you want to sync an entity's movement state (e.g. Position) without
 * changing its inputs, you can just registry.patch() with no changes.
 */
struct Input {
    enum Type : Uint8 { XUp, XDown, YUp, YDown, Jump, Crouch, Count, None };

    enum State : Uint8 { Released, Pressed };

    /** Holds the current state of the inputs, indexed by Input::Type. */
    using StateArr = std::bitset<6>;
    StateArr inputStates{};
};

template<typename S>
void serialize(S& serializer, Input& input)
{
    // Bit pack the input bitset.
    serializer.enableBitPacking(
        [&input](typename S::BPEnabledType& sbp) {
            sbp.ext(input.inputStates, bitsery::ext::StdBitset{});
        });

    // Note: We shouldn't need to align after bit packing (when the context ends,
    //       it'll auto-align), but measureSize() enables bit packing for 
    //       everything, so the context never ends and aligns itself.
    serializer.adapter().align();
}

} // namespace AM
