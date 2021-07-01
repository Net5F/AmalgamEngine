#pragma once

#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"

namespace AM
{
namespace SpriteEditor
{

/**
 * The left-hand panel on the main screen. Allows the user to manage the
 * project's sprite sheets.
 */
class SpritePanel : public AUI::Component
{
public:
    SpritePanel(AUI::Screen& inScreen);

    /**
     * Adds a MainThumbnail component to the spritesheetContainer, using the
     * given data.
     *
     * @param thumbPath  The path to the sprite sheet file to use as a
     *                   thumbnail, relative to AUI::Core::resourcePath.
     */
    void addSprite(const std::string& thumbPath);

    /**
     * Clears spritesheetContainer, removing all the sprite sheet components.
     */
    void clearSprites();

    bool onMouseButtonDown(SDL_MouseButtonEvent& event) override;

    void render(const SDL_Point& parentOffset = {}) override;

private:
    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spriteContainer;
};

} // End namespace SpriteEditor
} // End namespace AM
