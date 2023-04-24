#pragma once

#include "AUI/Window.h"
#include "AUI/Text.h"
#include "TitleButton.h"

namespace AM
{
namespace SpriteEditor
{
class UserInterface;
class TitleScreen;
class SpriteDataModel;

/**
 * The single window for the title screen.
 */
class TitleWindow : public AUI::Window
{
public:
    TitleWindow(UserInterface& inUserInterface,
                SpriteDataModel& inSpriteDataModel);

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    /** The user interface manager. Used for switching to the main screen. */
    UserInterface& userInterface;

    /** The sprite data for this project. Used for loading the user-selected
        file. */
    SpriteDataModel& spriteDataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text titleText;

    TitleButton newButton;

    TitleButton loadButton;

    AUI::Text errorText;
};

} // End namespace SpriteEditor
} // End namespace AM
