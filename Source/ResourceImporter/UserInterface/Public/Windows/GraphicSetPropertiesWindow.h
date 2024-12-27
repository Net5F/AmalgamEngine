#pragma once

#include "AUI/Window.h"
#include "AUI/Image.h"
#include "AUI/Text.h"
#include "MainTextInput.h"
#include "MainButton.h"
#include "LibraryItemData.h"
#include "GraphicSets.h"

namespace AM
{
struct BoundingBox;

namespace ResourceImporter
{
class DataModel;

/**
 * The properties window shown when the user loads a Floor, Floor Covering, Wall, 
 * or Object graphic set from the Library. Entity graphic sets use 
 * EntityGraphicPropertiesWindow.
 *
 * Allows the user to edit the active graphic set's properties.
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
    AUI::Text nameLabel;
    MainTextInput nameInput;

    /** Only used when a Floor graphic set is loaded. */
    MainButton setDefaultGraphicBoundsButton;

private:
    /**
     * If the new active item is a graphic set, loads it's data into this panel.
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

    /**
     * Sets the modelBounds of each sprite in this set to 
     * DEFAULT_FLOOR_SPRITE_BOUNDS.
     * Only used when a Floor graphic set is loaded.
     */
    void onSetDefaultGraphicBoundsButtonPressed();

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
