#ifndef INPUTCOMPONENT_H
#define INPUTCOMPONENT_H

#include "SharedDefs.h"
#include <array>

namespace AM
{
typedef std::array<Input::State, Input::Type::NumTypes> InputStateArr;

struct InputComponent
{
public:
    InputComponent()
    {
        inputStates.fill(Input::Released);
    }

    /** Holds the current state of the inputs, indexed by Input::Type. */
    InputStateArr inputStates;
};

} // namespace AM

#endif /* INPUTCOMPONENT_H  */
