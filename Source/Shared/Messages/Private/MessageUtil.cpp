#include "MessageUtil.h"

namespace AM
{

fb::InputState MessageUtil::convertToFbInputState(Input::State state)
{
    switch (state)
    {
        case Input::Invalid:
            return fb::InputState::Invalid;
            break;
        case Input::Pressed:
            return fb::InputState::Pressed;
            break;
        case Input::Released:
            return fb::InputState::Released;
            break;
        default:
            return fb::InputState::Invalid;
    }
}

Input::State MessageUtil::convertToAMInputState(fb::InputState state)
{
    switch (state)
    {
        case fb::InputState::Invalid:
            return Input::Invalid;
            break;
        case fb::InputState::Pressed:
            return Input::Pressed;
            break;
        case fb::InputState::Released:
            return Input::Released;
            break;
        default:
            return Input::State::Invalid;
    }
}

} // namespace AM
