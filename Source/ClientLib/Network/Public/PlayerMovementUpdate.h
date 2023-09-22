#pragma once

#include "Input.h"
#include "Position.h"
#include "Velocity.h"
#include "Rotation.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
namespace Client
{
/**
 * Holds updated movement state for the player entity.
 * 
 * This isn't an actual message that gets sent by the server, instead it gets
 * split out when we receive a ComponentUpdate with the relevant data.
 */
struct PlayerMovementUpdate {
    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The entity that this state belongs to. */
    entt::entity entity{entt::null};

    Input input{};
    Position position{};
    Velocity velocity{};
    Rotation rotation{};

    // std::visit-friendly interface.
    bool operator()(Input& inInput)
    {
        input = inInput;
        return true;
    }
    bool operator()(Position& inPosition)
    {
        position = inPosition;
        return true;
    }
    bool operator()(Velocity& inVelocity)
    {
        velocity = inVelocity;
        return true;
    }
    bool operator()(Rotation& inRotation)
    {
        rotation = inRotation;
        return true;
    }
    template<typename T>
    bool operator()(const T&)
    {
        return false;
    }
};

} // End namespace Client
} // End namespace AM
