#pragma once

#include "Position.h"
#include "PreviousPosition.h"
#include "TileExtent.h"
#include <SDL_rect.h>

namespace AM
{
/**
 * The player's camera viewport. Tracks where in the world the player is
 * currently viewing.
 */
struct Camera {
    enum MovementBehavior {
        // Camera will not move.
        Fixed,
        // Camera will center on its associated entity.
        CenterOnEntity
    };

    /** The camera's movement behavior. */
    MovementBehavior behavior{MovementBehavior::CenterOnEntity};

    /** Center of the camera's view, in world coordinates. */
    Position position{};

    /** The camera's previous position. Used for lerping in the renderer. */
    PreviousPosition prevPosition{};

    /** The camera's extent in screen space, calculated during the last render
        tick. */
    SDL_FRect extent{0, 0, 0, 0};

    /** The amount that this camera is zoomed in or out. 1.0 is no zoom. */
    float zoomFactor{1.0};

    /** How quickly the camera zooms. */
    float zoomSensitivity{0.1f};

    /**
     * Returns the tile extent that is in view of this camera.
     *
     * @param tileMapExtent  The tile map's bounds.
     */
    TileExtent getTileViewExtent(const TileExtent& tileMapExtent) const;
};

} // namespace AM
