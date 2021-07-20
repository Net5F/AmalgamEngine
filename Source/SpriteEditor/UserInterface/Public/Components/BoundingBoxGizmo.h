#pragma once

#include "AUI/Component.h"
#include "ScreenPoint.h"
#include <array>

namespace AM
{

class Position;
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
     * Note: This gizmo depends on having its logical extent set to match the
     *       sprite image that it will be overlaying.
     *
     * @param inActiveSprite  The active sprite's data.
     */
    void loadActiveSprite(SpriteStaticData* inActiveSprite);

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

    bool onMouseButtonDown(SDL_MouseButtonEvent& event) override;

    bool onMouseButtonUp(SDL_MouseButtonEvent& event) override;

    void onMouseMove(SDL_MouseMotionEvent& event) override;

protected:
    /**
     * Overridden to choose the proper resolution of texture to use.
     */
    bool refreshScaling() override;

private:
    /**
     * The list of our clickable controls.
     */
    enum class Control {
        None,
        Position,
        X,
        Y,
        Z
    };

    /**
     * Updates the active sprite's maxX and maxY bounds to match the given
     * mouse position.
     */
    void updatePositionBounds(const Position& mouseWorldPos);

    /**
     * Updates the active sprite's minX bound to match the given mouse position.
     */
    void updateXBounds(const Position& mouseWorldPos);

    /**
     * Updates the active sprite's minY bound to match the given mouse position.
     */
    void updateYBounds(const Position& mouseWorldPos);

    /**
     * Updates the active sprite's maxZ bound to match the given mouse position.
     */
    void updateZBounds(int mouseScreenYPos);

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
     * Moves the line extents to their proper screen position.
     */
    void moveLines(std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Moves the plane extents to their proper screen position.
     */
    void movePlanes(std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Renders the control rectangles.
     */
    void renderControls(const SDL_Point& childOffset);

    /**
     * Renders the line polygons.
     */
    void renderLines(const SDL_Point& childOffset);

    /**
     * Renders the plane polygons.
     */
    void renderPlanes(const SDL_Point& childOffset);

    /** Used to save updated sprite data. */
    MainScreen& mainScreen;

    /** The active sprite's data. */
    SpriteStaticData* activeSprite;

    /** A reasonable size for the control rectangles. */
    static constexpr int LOGICAL_RECT_SIZE = 12;

    /** The scaled size of the control rectangles. */
    int scaledRectSize;

    /** A reasonable width for the lines. */
    static constexpr int LOGICAL_LINE_WIDTH = 4;

    /** The scaled width of the lines. */
    int scaledLineWidth;

    // Controls (scaled extents, without parent offsets)
    /** The extent of the box position control, (maxX, maxY, minZ). */
    SDL_Rect positionControlExtent;
    /** positionControlExtent + offsets from the most recent render(). */
    SDL_Rect lastRenderedPosExtent;

    /** The extent of the x-axis box length control, (minX, maxY, minZ). */
    SDL_Rect xControlExtent;
    /** xControlExtent + offsets from the most recent render(). */
    SDL_Rect lastRenderedXExtent;

    /** The extent of the y-axis box length control, (maxX, minY, minZ). */
    SDL_Rect yControlExtent;
    /** yControlExtent + offsets from the most recent render(). */
    SDL_Rect lastRenderedYExtent;

    /** The extent of the z-axis box length control, (maxX, maxY, maxZ). */
    SDL_Rect zControlExtent;
    /** zControlExtent + offsets from the most recent render(). */
    SDL_Rect lastRenderedZExtent;

    // Lines
    SDL_Point xMinPoint;
    SDL_Point xMaxPoint;

    SDL_Point yMinPoint;
    SDL_Point yMaxPoint;

    SDL_Point zMinPoint;
    SDL_Point zMaxPoint;

    // Planes (each polygon takes 4 coordinates)
    std::array<Sint16, 12> planeXCoords;
    std::array<Sint16, 12> planeYCoords;

    /** Tracks which control, if any, is currently being held. */
    Control currentHeldControl;
};

} // End namespace SpriteEditor
} // End namespace AM
