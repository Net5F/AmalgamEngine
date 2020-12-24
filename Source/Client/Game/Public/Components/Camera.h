#pragma once

#include "Position.h"

namespace AM
{
namespace Client
{
/**
 * Stores camera position, bounds, and behavior.
 */
struct Camera {
public:
    /** Top left of the camera's view, in world coordinates. */
    Position position{};

    unsigned int width{0};
    unsigned int height{0};
};

} // namespace Client
} // namespace AM
