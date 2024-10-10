#pragma once

#include "AUI/Window.h"
#include "AUI/Text.h"
#include "TitleButton.h"

namespace AM
{
namespace ResourceImporter
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
    void onOpenButtonPressed();

    /** Used for switching to the main screen. */
    UserInterface& userInterface;
    /** Used for loading the user-selected file. */
    DataModel& dataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text titleText;

    AUI::Text directionText;

    TitleButton openButton;

    AUI::Text errorText;
};

} // End namespace ResourceImporter
} // End namespace AM
