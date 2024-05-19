#pragma once

#include "Position.h"
#include "PreviousPosition.h"
#include "BoundingBox.h"
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

    /** The position that the camera is pointing at.
        Since our camera always faces the same direction, it's more useful to 
        track the target than it is to track the camera's own position. */
    Position target{};

    /** The position that the camera was previously pointing at. Used for 
        lerping in the renderer. */
    PreviousPosition prevTarget{};

    /** The area that the camera is viewing, in world space.
        This is the total viewable area, at any zoom amount. */
    BoundingBox viewBounds{};

    /** The camera's view extent in screen space, calculated during the last 
        render tick in Renderer::getLerpedCamera(). */
    SDL_FRect screenExtent{0, 0, 0, 0};

    /** The amount that this camera is zoomed in or out. 1.0 is no zoom. */
    float zoomFactor{1.0};

    /** How quickly the camera zooms. */
    float zoomSensitivity{0.1f};

    /**
     * Returns the tile extent that is in view of this camera.
     *
     * @param mapTileExtent The tile map's bounds.
     */
    TileExtent getTileViewExtent(const TileExtent& mapTileExtent) const;
};

} // namespace AM
