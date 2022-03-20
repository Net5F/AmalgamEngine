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
class MainScreen;
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
    SpriteEditStage(AssetCache& inAssetCache, MainScreen& inScreen,
                    SpriteDataModel& inSpriteDataModel);

    /**
     * Loads the given sprite onto the stage.
     */
    void loadActiveSprite(Sprite* activeSprite);

    /**
     * Calls boundingBoxGizmo.refresh().
     */
    void refresh();

private:
    /** Used to load the active sprite's texture. */
    AssetCache& assetCache;

    /** Used to save/clear the active sprite when a sprite thumbnail is
        activated or deactivated. */
    MainScreen& mainScreen;

    /** Used to get the current working dir when displaying the sprite. */
    SpriteDataModel& spriteDataModel;

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
