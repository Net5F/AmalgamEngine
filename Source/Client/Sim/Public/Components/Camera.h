#pragma once

#include "Position.h"
#include "PreviousPosition.h"

namespace AM
{
namespace Client
{
/**
 * Stores camera position, bounds, and behavior.
 */
struct Camera {
public:
    enum MovementBehavior {
        // Camera will not move.
        Fixed,
        // Camera will center on its associated entity.
        CenterOnEntity
    };

    /** The camera's movement behavior. */
    MovementBehavior behavior;

    /** Top left of the camera's view, in world coordinates. */
    Position position{};

    /** The camera's previous position. Used for lerping in the renderer. */
    PreviousPosition prevPosition{};

    /** The camera's width in screen coordinates. */
    unsigned int width{0};
    /** The camera's height in screen coordinates. */
    unsigned int height{0};

    /** The amount that this camera is zoomed in or out. 1.0 is no zoom. */
    float zoomFactor{1.0};
};

} // namespace Client
} // namespace AM
