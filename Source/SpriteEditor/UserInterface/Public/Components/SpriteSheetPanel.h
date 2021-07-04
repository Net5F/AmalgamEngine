#pragma once

#include "SpriteSheet.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"
#include "AUI/Button.h"
#include "AddSheetDialog.h"

namespace AM
{
namespace SpriteEditor
{

class MainScreen;
class SpriteDataModel;

/**
 * The left-hand panel on the main screen. Allows the user to manage the
 * project's sprite sheets.
 */
class SpriteSheetPanel : public AUI::Component
{
public:
    SpriteSheetPanel(MainScreen& inScreen, SpriteDataModel& inSpriteDataModel);

    /**
     * Adds a MainThumbnail component to the spritesheetContainer, using the
     * given data.
     */
    void addSpriteSheet(const SpriteSheet& sheet);

    /**
     * Clears spritesheetContainer, removing all the sprite sheet components.
     */
    void clearSpriteSheets();

    bool onMouseButtonDown(SDL_MouseButtonEvent& event) override;

    void render(const SDL_Point& parentOffset = {}) override;

private:
    /** Used to open the confirmation dialog when removing a sheet. */
    MainScreen& mainScreen;

    /** Used to update the model when a sheet is removed. */
    SpriteDataModel& spriteDataModel;

    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spriteSheetContainer;

    AUI::Button remSheetButton;

    AUI::Button addSheetButton;

    AddSheetDialog addSheetDialog;
};

} // End namespace SpriteEditor
} // End namespace AM
