#pragma once

#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"
#include "AUI/Button.h"
#include "RemSheetDialog.h"
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
    SpriteSheetPanel(MainScreen& inScreen, SpriteDataModel& spriteDataModel);

    /**
     * Adds a MainThumbnail component to the spritesheetContainer, using the
     * given data.
     *
     * @param relPath  The path to the sprite sheet file, relative to
     *                 AUI::Core::resourcePath.
     */
    void addSpriteSheet(const std::string& relPath);

    /**
     * Clears spritesheetContainer, removing all the sprite sheet components.
     */
    void clearSpriteSheets();

    bool onMouseButtonDown(SDL_MouseButtonEvent& event) override;

    void render(const SDL_Point& parentOffset = {}) override;

private:
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spritesheetContainer;

    AUI::Button remSheetButton;

    AUI::Button addSheetButton;

    RemSheetDialog remSheetDialog;

    AddSheetDialog addSheetDialog;
};

} // End namespace SpriteEditor
} // End namespace AM
