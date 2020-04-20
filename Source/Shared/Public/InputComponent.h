#ifndef INPUTCOMPONENT_H
#define INPUTCOMPONENT_H

#include <array>

namespace AM
{

struct Input
{
    enum Type
    {
        None,
        Up,
        Down,
        Left,
        Right,
        Exit, // Exit the application.
        NumTypes
    };

    enum State
    {
        Invalid,
        Pressed,
        Released
    };

    Input(Type inType, State inState)
    : type(inType), state(inState)
    {
    }

    Type type;
    State state;
};

struct InputComponent
{
public:
    InputComponent()
    {
        inputStates.fill(Input::Released);
    }

    /** Holds the current state of the inputs. */
    std::array<Input::State, Input::NumTypes> inputStates;
};

} // namespace AM

#endif /* INPUTCOMPONENT_H  */
