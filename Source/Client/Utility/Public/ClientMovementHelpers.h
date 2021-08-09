#pragma once

#include "Input.h"
#include <array>

namespace AM
{
class Position;
namespace Client
{
class Sprite;

/**
 * Shared static functions for client-specific entity movement.
 */
class ClientMovementHelpers
{
public:
    /**
     * Moves a sprite's world bounds to the given position.
     */
    static void moveSpriteWorldBounds(Position& position, Sprite& sprite);
};

} // End namespace Client
} // End namespace AM
