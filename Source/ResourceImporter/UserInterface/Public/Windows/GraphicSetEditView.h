#pragma once

#include "LibraryItemData.h"
#include "LibraryListItem.h"
#include "GraphicSets.h"
#include "GraphicSetSlot.h"
#include "AUI/Window.h"
#include "AUI/Text.h"
#include "AUI/Image.h"
#include "AUI/VerticalGridContainer.h"

namespace AM
{
namespace ResourceImporter
{
class DataModel;
class LibraryWindow;

/**
 * The center stage shown when the user loads a Floor, Floor Covering, Wall, 
 * or Object graphic set from the Library. Entity graphic sets use 
 * EntityGraphicSetEditView.
 *
 * Allows the user to edit the active graphic set's graphic slots.
 */
class GraphicSetEditView : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    GraphicSetEditView(DataModel& inDataModel,
                       const LibraryWindow& inLibraryWindow);

private:
    /**
     * If the new active item is a graphic set, loads it's data onto this stage.
     */
    void onActiveLibraryItemChanged(const LibraryItemData& newActiveItem);

    /**
     * (If type/ID matches active set) Sets this stage back to its default
     * state.
     */
    void onGraphicSetRemoved(GraphicSet::Type type, Uint16 graphicSetID);

    /**
     * (If type/ID matches active set) Sets the given index to the given graphic.
     */
    void onGraphicSetSlotChanged(GraphicSet::Type type, Uint16 graphicSetID,
                                std::size_t index, GraphicID newGraphicID);

    /**
     * Loads the given graphic set's data onto this stage.
     */
    template<typename T>
    void loadActiveGraphicSet(GraphicSet::Type graphicSetType,
                             const T& newActiveGraphicSet);

    /**
     * Attempts to assign the currently selected library item to the 
     * given slot.
     */
    void onAssignButtonPressed(std::size_t slotIndex);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /**
     * Returns the appropriate top text for the given index.
     */
    std::string getSlotTopText(std::size_t graphicSetIndex);

    /**
     * Fills the given slot widget with the given graphic's image and name.
     */
    void fillSlotGraphicData(GraphicSetSlot& slot, GraphicID graphicID);

    /**
     * Fills the description text widgets with the appropriate strings, based
     * on the current activeGraphicSetType.
     */
    void fillDescriptionTexts();

    /** Used to get the current working dir when displaying the graphic. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    const LibraryWindow& libraryWindow;

    /** The active graphic set's type. */
    GraphicSet::Type activeGraphicSetType;

    /** The active graphic set's ID. */
    Uint16 activeGraphicSetID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;
    AUI::Text actionText;

    /** Holds this graphic set's graphics. */
    AUI::VerticalGridContainer graphicContainer;

    AUI::Text descText;
};

} // End namespace ResourceImporter
} // End namespace AM
