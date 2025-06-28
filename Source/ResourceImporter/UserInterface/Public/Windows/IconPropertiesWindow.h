#pragma once

#include "IconID.h"
#include "MainTextInput.h"
#include "LibraryItemData.h"
#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "AUI/Checkbox.h"

namespace AM
{
namespace ResourceImporter
{
class DataModel;

/**
 * The properties window shown when the user loads an icon from the Library.
 * Allows the user to edit the active icon's properties.
 */
class IconPropertiesWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    IconPropertiesWindow(DataModel& inDataModel);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in the EditorIcon class.
        See displayName field for more information. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

private:
    /**
     * If the new active item is an icon, loads it's data into this window.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If active icon was removed) Hides this window.
     */
    void onIconRemoved(IconID iconID);

    /**
     * (If ID matches active icon) Updates this panel with the active icon's
     * new properties.
     */
    void onIconDisplayNameChanged(IconID iconID,
                                  const std::string& newDisplayName);

    /** Used while setting user-inputted icon data. */
    DataModel& dataModel;

    /** The active icon's ID. */
    IconID activeIconID;

    /** The below functions are all for validating and saving the user's data
        when the text is committed. */
    void saveName();

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;
};

} // End namespace ResourceImporter
} // End namespace AM
