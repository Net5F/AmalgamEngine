#pragma once

#include "AUI/Screen.h"
#include "AUI/TiledImage.h"
#include "BoundingBoxGizmo.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
class MainScreen;
class SpriteStaticData;
class SpriteDataModel;

/**
 * The center stage on the main screen. Allows the user to edit a sprite's
 * bounding box.
 */
class SpriteEditStage : public AUI::Component
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
    void loadActiveSprite(SpriteStaticData* activeSprite);

    /**
     * Calls boundingBoxGizmo.refresh().
     */
    void refresh();

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& parentOffset = {}) override;

private:
    /** Used to load the active sprite's texture. */
    AssetCache& assetCache;

    /** Used to save/clear the active sprite when a sprite thumbnail is
        activated or deactivated. */
    MainScreen& mainScreen;

    /** Used to get the current working dir when displaying the sprite. */
    SpriteDataModel& spriteDataModel;

    /** Checkerboard image, tiled as the background for the loaded sprite. */
    AUI::TiledImage checkerboardImage;

    /** The sprite that is currently loaded onto the stage. */
    AUI::Image spriteImage;

    /** The gizmo for editing the sprite's bounding box. */
    BoundingBoxGizmo boundingBoxGizmo;
};

} // End namespace SpriteEditor
} // End namespace AM
