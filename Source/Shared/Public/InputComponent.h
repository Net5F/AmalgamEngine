#ifndef INPUTCOMPONENT_H
#define INPUTCOMPONENT_H

#include "SharedDefs.h"
#include <array>

namespace AM
{

struct InputComponent
{
public:
    InputComponent()
    {
        inputStates.fill(Input::Released);
    }

    /** Holds the current state of the inputs, indexed by Input::Type. */
    std::array<Input::State, Input::NumTypes> inputStates;
};

} // namespace AM

#endif /* INPUTCOMPONENT_H  */
