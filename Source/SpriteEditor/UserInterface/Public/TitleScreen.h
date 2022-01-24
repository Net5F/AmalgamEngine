#pragma once

#include "SpriteDataModel.h"
#include "AUI/Screen.h"
#include "AUI/Text.h"
#include "TitleButton.h"

namespace AM
{
class AssetCache;

namespace SpriteEditor
{
class UserInterface;

/**
 * The opening title screen that you see on app launch.
 */
class TitleScreen : public AUI::Screen
{
public:
    TitleScreen(UserInterface& inUserInterface, AssetCache& assetCache,
                SpriteDataModel& inSpriteDataModel);

    void render() override;

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    /** The user interface manager. Used for switching to the main screen. */
    UserInterface& userInterface;

    /** The sprite data for this project. Used for loading the user-selected
        file. */
    SpriteDataModel& spriteDataModel;

    //-------------------------------------------------------------------------
    // Child widgets
    //-------------------------------------------------------------------------
    AUI::Text titleText;

    TitleButton newButton;

    TitleButton loadButton;

    AUI::Text errorText;
};

} // End namespace SpriteEditor
} // End namespace AM
