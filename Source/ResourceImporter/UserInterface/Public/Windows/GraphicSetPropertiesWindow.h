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
 * The properties window shown when the user loads a graphic set from the
 * Library. Allows the user to edit the active graphic set's properties.
 */
class GraphicSetPropertiesWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    GraphicSetPropertiesWindow(DataModel& inDataModel);

    //-------------------------------------------------------------------------
    // Public child widgets
    //-------------------------------------------------------------------------
    /** All fields below directly match a data field in an
        Editor<type>GraphicSet class. */
    AUI::Text nameLabel;
    MainTextInput nameInput;

private:
    /**
     * If the new active item is a graphic, loads it's data into this panel.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If type/ID matches active set) Sets this panel back to its default
     * state.
     */
    void onGraphicSetRemoved(GraphicSet::Type type, Uint16 graphicSetID);

    /**
     * (If type/ID matches active set) Updates this panel with the active
     * graphic set's new properties.
     */
    void onGraphicSetDisplayNameChanged(GraphicSet::Type type, Uint16 graphicSetID,
                                       const std::string& newDisplayName);

    /**
     * Loads the given graphic set's data into this panel.
     */
    template<typename T>
    void loadActiveGraphicSet(GraphicSet::Type graphicSetType,
                             const T& newActiveGraphicSet);

    /** Used while setting user-inputted graphic set data. */
    DataModel& dataModel;

    /** The active graphic set's type. */
    GraphicSet::Type activeGraphicSetType;

    /** The active graphic set's ID. */
    Uint16 activeGraphicSetID;

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
