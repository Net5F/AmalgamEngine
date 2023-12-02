#pragma once

#include "AUI/Window.h"
#include "AUI/Button.h"

namespace AM
{
namespace ResourceImporter
{
class MainScreen;
class DataModel;

/**
 * The save button at the top of the screen, next to the properties.
 */
class SaveButtonWindow : public AUI::Window
{
public:
    SaveButtonWindow(MainScreen& inScreen, DataModel& inDataModel);

private:
    /** Used to open the confirmation dialog. */
    MainScreen& mainScreen;

    /** Used to save the model state. */
    DataModel& dataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Button saveButton;
};

} // End namespace ResourceImporter
} // End namespace AM
