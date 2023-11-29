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
class DataModel;

/**
 * The single window for the title screen.
 */
class TitleWindow : public AUI::Window
{
public:
    TitleWindow(UserInterface& inUserInterface, DataModel& inDataModel);

private:
    void onNewButtonPressed();

    void onLoadButtonPressed();

    /** Used for switching to the main screen. */
    UserInterface& userInterface;
    /** Used for loading the user-selected file. */
    DataModel& dataModel;

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
