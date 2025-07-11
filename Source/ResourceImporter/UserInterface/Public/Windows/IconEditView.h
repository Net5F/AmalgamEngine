#pragma once

#include "LibraryItemData.h"
#include "AUI/Window.h"
#include "AUI/Screen.h"
#include "AUI/Text.h"
#include "AUI/Image.h"

namespace AM
{
namespace ResourceImporter
{
class DataModel;

/**
 * The center stage shown when the user loads an icon from the Library.
 */
class IconEditView : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    IconEditView(DataModel& inDataModel);

private:
    /**
     * If the new active item is a icon, loads it's data onto this stage.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If active icon was removed) Hides this view.
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

} // End namespace ResourceImporter
} // End namespace AM
