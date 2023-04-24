#pragma once

#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "BoundingBoxGizmo.h"

namespace AM
{
namespace SpriteEditor
{
struct Sprite;
class SpriteDataModel;

/**
 * The center stage on the main screen. Allows the user to edit a sprite's
 * bounding box.
 */
class SpriteEditStage : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    SpriteEditStage(SpriteDataModel& inSpriteDataModel);

private:
    /**
     * Loads the new active sprite's data onto the stage.
     */
    void onActiveSpriteChanged(unsigned int newSpriteID,
                               const Sprite& newSprite);

    /**
     * (If active sprite was removed) Sets activeSprite to invalid and returns
     * the stage to its default state.
     */
    void onSpriteRemoved(unsigned int spriteID);

    /** Used to get the current working dir when displaying the sprite. */
    SpriteDataModel& spriteDataModel;

    /** The active sprite's ID. */
    unsigned int activeSpriteID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    /** Checkerboard image, tiled as the background for the loaded sprite. */
    AUI::Image checkerboardImage;

    /** The sprite that is currently loaded onto the stage. */
    AUI::Image spriteImage;

    /** The gizmo for editing the sprite's bounding box. */
    BoundingBoxGizmo boundingBoxGizmo;
};

} // End namespace SpriteEditor
} // End namespace AM
