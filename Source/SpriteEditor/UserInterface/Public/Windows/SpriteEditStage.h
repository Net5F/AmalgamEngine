#pragma once

#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Image.h"
#include "LibraryItemData.h"
#include "BoundingBoxGizmo.h"

namespace AM
{
namespace SpriteEditor
{
class SpriteDataModel;
struct EditorSprite;

/**
 * The center stage shown when the user loads a sprite from the Library.
 * Allows the user to edit the active sprite's bounding box.
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
     * If the new active item is a sprite, loads it's data onto this stage.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If active sprite was removed) Sets activeSprite to invalid and returns
     * the stage to its default state.
     */
    void onSpriteRemoved(int spriteID);

    /** Used to get the current working dir when displaying the sprite. */
    SpriteDataModel& spriteDataModel;

    /** The active sprite's ID. */
    int activeSpriteID;

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
