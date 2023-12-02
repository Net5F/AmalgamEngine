#pragma once

#include "LibraryItemData.h"
#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Text.h"
#include "AUI/Image.h"

namespace AM
{
namespace SpriteEditor
{
class DataModel;

/**
 * The center stage shown when the user loads an icon from the Library.
 */
class IconEditStage : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    IconEditStage(DataModel& inDataModel);

private:
    /**
     * If the new active item is a icon, loads it's data onto this stage.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If active icon was removed) Sets activeIcon to invalid and returns
     * the stage to its default state.
     */
    void onIconRemoved(IconID iconID);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /** Used to get the current working dir when displaying the icon. */
    DataModel& dataModel;

    /** The active icon's ID. */
    IconID activeIconID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;

    /** Checkerboard image, tiled as the background for the loaded icon. */
    AUI::Image checkerboardImage;

    /** The icon that is currently loaded onto the stage. */
    AUI::Image iconImage;

    AUI::Text descText;
};

} // End namespace SpriteEditor
} // End namespace AM
