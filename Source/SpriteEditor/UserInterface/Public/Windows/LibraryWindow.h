#pragma once

#include "SpriteSheet.h"
#include "AUI/Window.h"
#include "AUI/Text.h"
#include "AUI/Image.h"
#include "AUI/FlowContainer.h"
#include "AUI/Button.h"

namespace AM
{
namespace SpriteEditor
{
class MainScreen;
class SpriteDataModel;

// TODO: Make this obtain focus and deselect all selected thumbnails when
//       focus is lost.
/**
 * The left-side panel on the main screen. Allows the user to manage the
 * project's sprite sheets.
 */
class LibraryWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    LibraryWindow(MainScreen& inScreen, SpriteDataModel& inSpriteDataModel);

private:
    /** Used to open the confirmation dialog when removing a sheet. */
    MainScreen& mainScreen;

    /** Used to update the model when a sheet is removed. */
    SpriteDataModel& spriteDataModel;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;

    AUI::FlowContainer categoryContainer;

    AUI::Button newButton;
};

} // End namespace SpriteEditor
} // End namespace AM
