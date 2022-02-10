#pragma once

#include "SpriteSheet.h"
#include "Sprite.h"
#include "AUI/Window.h"
#include "AUI/Button.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
class MainScreen;
class SpriteDataModel;

/**
 * The save button at the top of the screen, next to the properties.
 */
class SaveButtonWindow : public AUI::Window
{
public:
    SaveButtonWindow(AssetCache& inAssetCache, MainScreen& inScreen
                     , SpriteDataModel& inSpriteDataModel);

private:
    /** Used to load the button's textures. */
    AssetCache& assetCache;

    /** Used to open the confirmation dialog. */
    MainScreen& mainScreen;

    /** Used to save the model state. */
    SpriteDataModel& spriteDataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    /** The save button at the top of the screen, next to the properties. */
    AUI::Button saveButton;
};

} // End namespace SpriteEditor
} // End namespace AM
