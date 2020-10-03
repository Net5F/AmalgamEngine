#pragma once

#include "GameDefs.h"

namespace AM
{

struct InputComponent
{
public:
    //--------------------------------------------------------------------------
    // Replicated data
    //--------------------------------------------------------------------------
    /** Holds the current state of the inputs, indexed by Input::Type. */
    InputStateArr inputStates = {};
};

template <typename S>
void serialize(S& serializer, InputComponent& inputComponent)
{
    serializer.container1b(inputComponent.inputStates);
}

} // namespace AM
