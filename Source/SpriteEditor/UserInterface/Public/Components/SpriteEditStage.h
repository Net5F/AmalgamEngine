#pragma once

#include "AUI/Screen.h"
#include "AUI/TiledImage.h"

namespace AM
{
namespace SpriteEditor
{

class MainScreen;
class SpriteStaticData;

/**
 * The center stage on the main screen. Allows the user to edit a sprite's
 * bounding box.
 */
class SpriteEditStage : public AUI::Component
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteEditStage(MainScreen& inScreen);

    /**
     * Loads the MainScreen's current active sprite onto the stage.
     */
    void loadActiveSprite();

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void render(const SDL_Point& parentOffset = {}) override;

private:
    /** Used to save/clear the active sprite when a sprite thumbnail is
        activated or deactivated. */
    MainScreen& mainScreen;

    /** Checkerboard image, tiled as the background for the loaded sprite. */
    AUI::TiledImage checkerboardImage;

    /** The sprite that is currently loaded onto the stage. */
    AUI::Image spriteImage;
};

} // End namespace SpriteEditor
} // End namespace AM
