#pragma once

#include "Vector3.h"
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
 * A gizmo that allows the user to draw a 3D point on their 2D sprites.
 */
class PointGizmo : public AUI::Widget
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    PointGizmo(const SDL_Rect& inLogicalExtent);

    virtual ~PointGizmo() = default;

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
     * Sets this gizmo to match newPoint.
     */
    void setPoint(const Vector3& newPoint);
    
    /**
     * Returns the sprite image extent that was set by the last call to 
     * setSpriteImageSize(), centered within this widget.
     */
    const SDL_Rect& getLogicalCenteredSpriteExtent() const;

    //-------------------------------------------------------------------------
    // Callback registration
    //-------------------------------------------------------------------------
    /**
     * @param inOnPointUpdated A callback that expects the updated point.
     */
    void setOnPointUpdated(
        std::function<void(const Vector3& updatedPoint)> inOnPointUpdated);

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
    
    void refreshScaling();

    /**
     * Updates stageWorldExtent to match the current stageOrigin and widget 
     * size.
     */
    void updateStageExtent();

    /**
     * Refreshes this widget's graphics  to match its internal state.
     */
    void refreshGraphics();

    /**
     * Updates the point to match the given mouse position.
     */
    void updatePoint(const Position& mouseWorldPos);

    /** The value of AUI::Core::actualScreenSize that was used the last time
        this widget updated its layout. Used to detect when the UI scale
        changes, so we can resize our controls. */
    AUI::ScreenResolution lastUsedScreenSize;

    /** The point that this gizmo is representing. */
    Vector3 point;

    /** If false, this widget should ignore all interactions and render as 
        semi-transparent. */
    bool isEnabled;

    /** A reasonable size for the control rectangles. */
    static constexpr int LOGICAL_RECT_SIZE{12};

    /** The scaled size of the control rectangles. */
    int scaledRectSize;

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
    /** The extent of the point position control. */
    SDL_Rect pointControlExtent;

    /** If true, the point control is currently being held. */
    bool currentlyHeld;

    std::function<void(const Vector3&)> onPointUpdated;
};

} // End namespace ResourceImporter
} // End namespace AM
