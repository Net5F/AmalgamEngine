#pragma once

#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"
#include "AUI/Button.h"
#include "RemSheetDialog.h"

namespace AM
{

/**
 * The left-hand panel on the main screen. Allows the user to manage the
 * project's sprite sheets.
 */
class SpriteSheetPanel : public AUI::Component
{
public:
    SpriteSheetPanel(AUI::Screen& screen);

    bool onMouseButtonDown(SDL_MouseButtonEvent& event) override;

    void render(const SDL_Point& parentOffset = {}) override;

private:
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spritesheetContainer;

    AUI::Button remSheetButton;

    AUI::Button addSheetButton;

    RemSheetDialog remSheetDialog;
};

} // End namespace AM
