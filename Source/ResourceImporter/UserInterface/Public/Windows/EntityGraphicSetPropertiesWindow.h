#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "MainTextInput.h"
#include "LibraryItemData.h"
#include "GraphicSets.h"

namespace AM
{
struct BoundingBox;

namespace ResourceImporter
{
class DataModel;

/**
 * The properties window shown when the user loads an Entity graphic set.
 *
 * Allows the user to edit the active graphic set's properties.
 *
 * Note: This is separate from GraphicSetPropertiesWindow because entity 
 *       graphic sets have significant differences from the others.
 */
class EntityGraphicSetPropertiesWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    EntityGraphicSetPropertiesWindow(DataModel& inDataModel);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in the 
        EditorEntityGraphicSet class. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

private:
    /**
     * If the new active item is an entity graphic set, loads it's data into 
     * this panel.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If ID matches active set) Sets this panel back to its default state.
     */
    void onEntityRemoved(EntityGraphicSetID graphicSetID);

    /**
     * (If ID matches active set) Updates this panel with the active graphic 
     * set's new properties.
     */
    void onEntityDisplayNameChanged(EntityGraphicSetID graphicSetID,
                                    const std::string& newDisplayName);

    /** Used while setting user-inputted graphic set data. */
    DataModel& dataModel;

    /** The active graphic set's ID. */
    EntityGraphicSetID activeGraphicSetID;

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
