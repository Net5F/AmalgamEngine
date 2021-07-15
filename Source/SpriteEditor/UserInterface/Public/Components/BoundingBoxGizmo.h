#pragma once

#include "AUI/Component.h"
#include "ScreenPoint.h"
#include <vector>

namespace AM
{

class BoundingBox;

namespace SpriteEditor
{

class MainScreen;
class SpriteStaticData;

/**
 * A confirmation dialog with header text, body text, and confirm/cancel
 * buttons.
 */
class BoundingBoxGizmo : public AUI::Component
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    BoundingBoxGizmo(MainScreen& inScreen);

    virtual ~BoundingBoxGizmo() = default;

    /**
     * Loads the given active sprite's data into this gizmo.
     *
     * @param inActiveSprite  The active sprite's data.
     * @param inActiveSpriteUiExtent  The sprite's extent on screen, relative
     *                                to the stage.
     */
    void loadActiveSprite(SpriteStaticData* inActiveSprite
                          , SDL_Rect inActiveSpriteUiExtent);

    /**
     * Refreshes this component's UI with the data from the currently set
     * active sprite.
     * Errors if activeSprite is nullptr.
     */
    void refresh();

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& parentOffset = {}) override;

protected:
    /**
     * Overridden to choose the proper resolution of texture to use.
     */
    bool refreshScaling() override;

private:
    /**
     * Transforms the positions in the given sprite's bounding box from world
     * space to screen space, scales them to the current UI scaling, and
     * offsets them to overlay the given sprite.
     *
     * The finished points are pushed into the given vector in the order:
     *     (minX, maxY, minZ), (maxX, maxY, minZ), (maxX, minY, minZ),
     *     (minX, maxY, maxZ), (maxX, maxY, maxZ), (maxX, minY, maxZ),
     *     (minX, minY, maxZ)
     */
    void calcOffsetScreenPoints(std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Moves the control extents to their proper screen position.
     */
    void moveControls(std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Renders the control rectangles.
     */
    void renderControls(const SDL_Point& childOffset);

    /** Used to save updated sprite data. */
    MainScreen& mainScreen;

    /** The active sprite's data. */
    SpriteStaticData* activeSprite;

    /** The extent of the active sprite in the UI. */
    SDL_Rect activeSpriteUiExtent;

    /** A reasonable size for the control rectangles. */
    static constexpr int LOGICAL_RECT_SIZE = 10;

    /** The scaled size of the control rectangles. */
    int scaledRectSize;

    /** The extent of the box position control, (maxX, maxY, minZ). */
    SDL_Rect positionControlExtent;

    /** The extent of the x-axis box length control, (minX, maxY, minZ). */
    SDL_Rect xControlExtent;

    /** The extent of the y-axis box length control, (maxX, minY, minZ). */
    SDL_Rect yControlExtent;

    /** The extent of the z-axis box length control, (maxX, maxY, maxZ). */
    SDL_Rect zControlExtent;
};

} // End namespace SpriteEditor
} // End namespace AM
