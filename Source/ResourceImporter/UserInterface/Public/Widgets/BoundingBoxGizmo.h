#pragma once

#include "BoundingBox.h"
#include "LibraryItemData.h"
#include "AUI/Widget.h"
#include "AUI/ScreenResolution.h"
#include <array>
#include <functional>

namespace AM
{
struct Position;

namespace ResourceImporter
{
struct EditorSprite;

/**
 * A gizmo that allows the user to draw 3D bounding boxes on their 2D sprites.
 */
class BoundingBoxGizmo : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    BoundingBoxGizmo(const SDL_Rect& inLogicalExtent);

    virtual ~BoundingBoxGizmo() = default;

    /**
     * Enabled this gizmo, allowing it to respond to user input.
     */
    void enable();

    /**
     * Disables this gizmo, causing it to ignore user input and render as 
     * semi-transparent.
     */
    void disable();

    /**
     * Sets the size of the sprite image that this gizmo is drawing over.
     * The resulting extent will be centered within this widget.
     */
    void setSpriteImageSize(int logicalSpriteWidth, int logicalSpriteHeight);

    /**
     * Sets the stage's screen-space origin offset, relative to the top left of 
     * the image.
     */
    void setStageOrigin(const SDL_Point& inLogicalStageOrigin);

    /**
     * Sets this gizmo to match newBoundingBox.
     */
    void setBoundingBox(const BoundingBox& newBoundingBox);
    
    /**
     * Returns the sprite image extent that was set by the last call to 
     * setSpriteImageSize(), centered within this widget.
     */
    const SDL_Rect& getLogicalCenteredSpriteExtent() const;

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnBoundingBoxUpdated A callback that expects the updated 
     *                               bounding box.
     */
    void setOnBoundingBoxUpdated(
        std::function<void(const BoundingBox& updatedBounds)> inOnBoundingBoxUpdated);

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void setLogicalExtent(const SDL_Rect& inLogicalExtent) override;

    /**
     * If the UI scaling has changed, refreshes our controls.
     */
    void arrange(const SDL_Point& startPosition,
                 const SDL_Rect& availableExtent,
                 AUI::WidgetLocator* widgetLocator) override;

    void render(const SDL_Point& windowTopLeft) override;

    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType,
                                 const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseUp(AUI::MouseButtonType buttonType,
                               const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseMove(const SDL_Point& cursorPosition) override;

private:
    /** The base transparency value for a selected gizmo. */
    static constexpr float BASE_ALPHA{255};

    /** How opaque a disabled gizmo will be. */
    static constexpr float DISABLED_ALPHA_FACTOR{0.25f};

    /** How opaque the sides of the bounding box will be. */
    static constexpr float PLANE_ALPHA_FACTOR{0.5f};

    /**
     * The list of our clickable controls.
     */
    enum class Control { None, Position, X, Y, Z };
    
    void refreshScaling();

    /**
     * Updates stageWorldExtent to match the current stageOrigin and widget 
     * size.
     */
    void updateStageExtent();

    /**
     * Refreshes this widget's graphics (controls/lines/planes) to match its
     * internal state.
     */
    void refreshGraphics();

    /**
     * Updates the maxX and maxY bounds to match the given mouse position.
     */
    void updatePositionBounds(const Position& mouseWorldPos);

    /**
     * Updates the minX bound to match the given mouse position.
     */
    void updateXBounds(const Position& mouseWorldPos);

    /**
     * Updates the minY bound to match the given mouse position.
     */
    void updateYBounds(const Position& mouseWorldPos);

    /**
     * Updates the maxZ bound to match the given mouse position.
     */
    void updateZBounds(int mouseScreenYPos);

    /**
     * Transforms the vertices in the current boundingBox from world space 
     * to screen space, scales them to the current UI scaling, and offsets 
     * them using the current offsets.
     *
     * The finished points are pushed into the given vector in the order:
     *     (minX, maxY, minZ), (maxX, maxY, minZ), (maxX, minY, minZ),
     *     (minX, maxY, maxZ), (maxX, maxY, maxZ), (maxX, minY, maxZ),
     *     (minX, minY, maxZ)
     */
    void
        calcBoundingBoxScreenPoints(std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Moves the control extents to their proper screen position.
     */
    void moveControls(std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Moves the line points to their proper screen position.
     */
    void moveLines(std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Moves the plane coords to their proper screen position.
     */
    void movePlanes(std::vector<SDL_Point>& boundsScreenPoints);

    /**
     * Renders the control rectangles.
     */
    void renderControls(const SDL_Point& windowTopLeft);

    /**
     * Renders the line polygons.
     */
    void renderLines(const SDL_Point& windowTopLeft);

    /**
     * Renders the plane polygons.
     */
    void renderPlanes(const SDL_Point& windowTopLeft);

    /** The value of AUI::Core::actualScreenSize that was used the last time
        this widget updated its layout. Used to detect when the UI scale
        changes, so we can resize our controls. */
    AUI::ScreenResolution lastUsedScreenSize;

    /** The bounding box that this gizmo is representing. */
    BoundingBox boundingBox;

    /** If false, this widget should ignore all interactions and render as 
        semi-transparent. */
    bool isEnabled;

    /** A reasonable size for the control rectangles. */
    static constexpr int LOGICAL_RECT_SIZE{12};

    /** The scaled size of the control rectangles. */
    int scaledRectSize;

    /** A reasonable width for the lines. */
    static constexpr int LOGICAL_LINE_WIDTH{4};

    /** The scaled width of the lines. */
    int scaledLineWidth;

    /** Sets the extent where the sprite image will be placed, relative to the
        top left of this widget. */
    SDL_Rect logicalSpriteImageExtent;

    /** The stage's screen-space origin offset, relative to the top left of 
        the image. */
    SDL_Point logicalStageOrigin;

    /** The stage's world-space extent.
        We limit this extent to the edges of the sprite image. */
    BoundingBox stageWorldExtent;

    // Controls (scaled extents, without parent offsets)
    /** The extent of the box position control, (maxX, maxY, minZ). */
    SDL_Rect positionControlExtent;

    /** The extent of the x-axis box length control, (minX, maxY, minZ). */
    SDL_Rect xControlExtent;

    /** The extent of the y-axis box length control, (maxX, minY, minZ). */
    SDL_Rect yControlExtent;

    /** The extent of the z-axis box length control, (maxX, maxY, maxZ). */
    SDL_Rect zControlExtent;

    // Lines
    SDL_Point lineXMinPoint;
    SDL_Point lineXMaxPoint;

    SDL_Point lineYMinPoint;
    SDL_Point lineYMaxPoint;

    SDL_Point lineZMinPoint;
    SDL_Point lineZMaxPoint;

    // Planes (each polygon takes 4 coordinates)
    std::array<Sint16, 12> planeXCoords;
    std::array<Sint16, 12> planeYCoords;

    /** Tracks which control, if any, is currently being held. */
    Control currentHeldControl;

    std::function<void(const BoundingBox&)> onBoundingBoxUpdated;
};

} // End namespace ResourceImporter
} // End namespace AM
