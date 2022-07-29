#pragma once

#include "AUI/Widget.h"
#include "ScreenPoint.h"
#include <array>

namespace AM
{
struct Position;
struct BoundingBox;

namespace SpriteEditor
{
struct Sprite;
class SpriteDataModel;

/**
 * A gizmo that allows the user to draw 3D bounding boxes for their 2D sprites.
 */
class BoundingBoxGizmo : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    BoundingBoxGizmo(SpriteDataModel& inSpriteDataModel,
                     unsigned int inSpriteID,
                     unsigned int inModelBoundsIndex);

    virtual ~BoundingBoxGizmo() = default;

    /**
     * Sets whether the gizmo is selected or not.
     * Deselected gizmos are more transparent and hide their controls.
     */
    void setIsSelected(bool inIsSelected);
    bool getIsSelected();

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render() override;

    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                                 const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseUp(AUI::MouseButtonType buttonType,
                               const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseDoubleClick(AUI::MouseButtonType buttonType,
                                   const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseMove(const SDL_Point& cursorPosition) override;

protected:
    /**
     * Overridden to choose the proper resolution of texture to use.
     */
    bool refreshScaling() override;

private:
    /** The base transparency value for a selected gizmo. */
    static constexpr float BASE_ALPHA{255};

    /** How opaque the sides of the bounding box will be. */
    static constexpr float PLANE_ALPHA_FACTOR{0.6f};

    /** How opaque an unselected gizmo will be. */
    static constexpr float UNSELECTED_ALPHA_FACTOR{0.65f};

    /**
     * The list of this gizmo's clickable targets.
     */
    enum class HitTarget {
        None,
        Box,
        PositionControl,
        XControl,
        YControl,
        ZControl
    };

    /**
     * Updates this gizmo with the active sprite's new properties.
     * Note: This gizmo depends on having its logical extent set to match the
     *       sprite image that it will be overlaying.
     */
    void onSpriteModelBoundsChanged(unsigned int inSpriteID,
                                    unsigned int changedBoundsIndex,
                                    const BoundingBox& newModelBounds);

    /**
     * Refreshes this widget's UI with the given active sprite's data.
     */
    void refresh(const Sprite& activeSprite);

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
    void calcOffsetScreenPoints(const Sprite& activeSprite,
                                std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Sets lastEnclosingPolygon using the given points.
     */
    void setLastEnclosingPolygon(std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Hit tests the given mouse cursor position against the current box and 
     * control positions.
     *
     * @return A target if one was hit, else HitTarget::None.
     */
    HitTarget hitTest(const SDL_Point& cursorPosition);

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
    void renderControls();

    /**
     * Renders the line polygons.
     */
    void renderLines();

    /**
     * Renders the plane polygons.
     */
    void renderPlanes();

    /** Used while setting user-inputted sprite data. */
    SpriteDataModel& spriteDataModel;

    /** The ID of the sprite that this gizmo is associated with. */
    unsigned int spriteID;

    /** The index within the associated sprite's modelBounds vector of the box 
        that this gizmo is associated with. */
    unsigned int modelBoundsIndex;

    /** A reasonable size for the control rectangles. */
    static constexpr int LOGICAL_RECT_SIZE{12};

    /** The scaled size of the control rectangles. */
    int scaledRectSize;

    /** A reasonable width for the lines. */
    static constexpr int LOGICAL_LINE_WIDTH{4};

    /** The scaled width of the lines. */
    int scaledLineWidth;

    /** Tracks whether this gizmo is selected or not. */
    bool isSelected;

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
    HitTarget currentHeldControl;

    /** Holds a screen-space polygon that encloses this gizmo's bounding box.
        Calc'd during refresh(). Used in hit testing.
        The points in this array start at the topmost vertex and proceed 
        clockwise, and are relative to the top left of the screen. */
    std::array<SDL_Point, 6> lastEnclosingPolygon;
};

} // End namespace SpriteEditor
} // End namespace AM
