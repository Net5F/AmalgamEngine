#pragma once

#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/TiledImage.h"
#include "BoundingBoxGizmo.h"
#include <unordered_map>

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
struct Sprite;
class SpriteDataModel;

/**
 * The center stage on the main screen. Allows the user to edit a sprite's
 * bounding box.
 */
class SpriteEditStage : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteEditStage(AssetCache& inAssetCache,
                    SpriteDataModel& inSpriteDataModel);

private:
    /**
     * Loads the new active sprite's data onto the stage.
     */
    void onActiveSpriteChanged(unsigned int newActiveSpriteID,
                               unsigned int newActiveModelBoundsIndex,
                               const Sprite& newSprite);

    /**
     * Adds a BoundingBoxGizmo to reflect the new bounding box.
     */
    void onSpriteModelBoundsAdded(unsigned int spriteID,
                                  unsigned int addedBoundsIndex,
                                  const BoundingBox& newModelBounds);

    /**
     * Removes the BoundingBoxGizmo associated with the old bounding box.
     */
    void onSpriteModelBoundsRemoved(unsigned int spriteID,
                                    unsigned int removedBoundsIndex);

    /**
     * Brings the gizmo associated with the newly-activated bounds to the front 
     * and sets it as active.
     */
    void onActiveSpriteModelBoundsChanged(unsigned int newActiveModelBoundsIndex,
                    const BoundingBox& newActiveModelBounds);

    /**
     * (If active sprite was removed) Sets activeSprite to invalid and returns
     * the stage to its default state.
     */
    void onSpriteRemoved(unsigned int spriteID);

    /**
     * Selects the gizmo associated with the given model bounds index, and 
     * deselects all other gizmos.
     */
    std::vector<std::reference_wrapper<AUI::Widget>>::iterator
        selectGizmoAtIndex(unsigned int modelBoundsIndex);

    /** Used to load the active sprite's texture. */
    AssetCache& assetCache;

    /** Used to get the current working dir when displaying the sprite. */
    SpriteDataModel& spriteDataModel;

    /** The active sprite's ID. */
    unsigned int activeSpriteID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    /** Checkerboard image, tiled as the background for the loaded sprite. */
    AUI::TiledImage checkerboardImage;

    /** The sprite that is currently loaded onto the stage. */
    AUI::Image spriteImage;

    /** The gizmos for editing the sprite's bounding box. */
    std::vector<BoundingBoxGizmo> boundingBoxGizmos;
};

} // End namespace SpriteEditor
} // End namespace AM
