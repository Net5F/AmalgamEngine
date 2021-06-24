#pragma once

#include "SpriteDataModel.h"
#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "TitleButton.h"

namespace AM
{
namespace SpriteEditor
{

class UserInterface;

/**
 * The opening title screen that you see on app launch.
 */
class TitleScreen : public AUI::Screen
{
public:
    TitleScreen(UserInterface& inUserInterface, SpriteDataModel& inSpriteDataModel);

    void render() override;

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    /** The user interface manager. Used for switching to the main screen. */
    UserInterface& userInterface;

    /** The sprite data for this project. Used for loading the user-selected
        file. */
    SpriteDataModel& spriteDataModel;

    AUI::Image background;

    TitleButton newButton;

    TitleButton loadButton;

    AUI::Text errorText;
};

} // End namespace SpriteEditor
} // End namespace AM
