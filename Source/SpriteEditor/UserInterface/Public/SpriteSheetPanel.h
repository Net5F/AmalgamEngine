#pragma once

#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"
#include "AUI/Button.h"

namespace AM
{

/**
 * The left-hand panel on the main screen. Allows the user to add sprite sheets
 * to the project.
 */
class SpriteSheetPanel : public AUI::Component
{
public:
    SpriteSheetPanel(AUI::Screen& screen);

    void render(const SDL_Point& parentOffset = {}) override;

private:
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spritesheetContainer;

    AUI::Button remSheetButton;

    AUI::Button addSheetButton;
};

} // End namespace AM
