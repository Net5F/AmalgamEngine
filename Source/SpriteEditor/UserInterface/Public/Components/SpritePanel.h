#pragma once

#include "SpriteSheet.h"
#include "SpriteStaticData.h"
#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"

namespace AM
{
namespace SpriteEditor
{

class MainScreen;

/**
 * The left-hand panel on the main screen. Allows the user to manage the
 * project's sprite sheets.
 */
class SpritePanel : public AUI::Component
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpritePanel(MainScreen& inScreen);

    /**
     * Adds a MainThumbnail component to the spriteContainer, using the
     * given data.
     */
    void addSprite(const SpriteSheet& sheet, const SpriteStaticData& sprite);

    /**
     * Clears spritesheetContainer, removing all the sprite sheet components.
     */
    void clearSprites();

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& parentOffset = {}) override;

private:
    /** Used to save/clear the active sprite when a sprite thumbnail is
        activated or deactivated. */
    MainScreen& mainScreen;

    AUI::Image backgroundImage;

    AUI::VerticalGridContainer spriteContainer;
};

} // End namespace SpriteEditor
} // End namespace AM
