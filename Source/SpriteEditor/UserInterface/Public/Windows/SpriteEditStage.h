#pragma once

#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/TiledImage.h"
#include "BoundingBoxGizmo.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
class Sprite;
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
    void onActiveSpriteChanged(unsigned int newSpriteID, const Sprite& newSprite);

    /**
     * (If active sprite was removed) Sets activeSprite to invalid and returns
     * the stage to its default state.
     */
    void onSpriteRemoved(unsigned int spriteID);

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

    /** The gizmo for editing the sprite's bounding box. */
    BoundingBoxGizmo boundingBoxGizmo;
};

} // End namespace SpriteEditor
} // End namespace AM
