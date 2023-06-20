#pragma once

#include "AUI/Window.h"
#include "AUI/Button.h"

namespace AM
{
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
    SaveButtonWindow(MainScreen& inScreen, SpriteDataModel& inSpriteDataModel);

private:
    /** Used to open the confirmation dialog. */
    MainScreen& mainScreen;

    /** Used to save the model state. */
    SpriteDataModel& spriteDataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Button saveButton;
};

} // End namespace SpriteEditor
} // End namespace AM
