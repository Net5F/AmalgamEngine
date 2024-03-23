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
 * The center stage shown when the user loads an Entity graphic set.
 *
 * Allows the user to edit the active graphic set's graphic slots.
 *
 * Note: This is separate from GraphicSetEditStage because entity graphic sets 
 *       have significant differences from the others.
 */
class EntityGraphicSetEditStage : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    EntityGraphicSetEditStage(DataModel& inDataModel,
                       const LibraryWindow& inLibraryWindow);

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
     * (If ID matches active set) Sets the given graphic type to the given 
     * graphic, adding or removing slot widgets as necessary.
     */
    void onEntitySlotChanged(EntityGraphicSetID graphicSetID,
                             EntityGraphicType graphicType,
                             GraphicID newGraphicID);

    /**
     * Attempts to assign the currently selected library item to the 
     * given graphic type's slot.
     */
    void onAssignButtonPressed(EntityGraphicType graphicType);

    /**
     * Styles the given text.
     */
    void styleText(AUI::Text& text);

    /**
     * Fills graphicContainer with empty slot widgets for each EntityGraphicType.
     */
    void initGraphicContainer();

    /**
     * Fills the given slot widget with the given graphic's image and name.
     */
    void fillSlotGraphicData(GraphicSetSlot& slot, GraphicID graphicID);

    /** Converts an entity graphic type into the index within graphicContainer 
        that the associated slot widget can be found. */
    constexpr std::size_t toIndex(EntityGraphicType graphicType);

    /** Converts a graphicContainer index into the entity graphic type 
        associated with that index. */
    constexpr EntityGraphicType toEntityGraphicType(Uint8 index);

    /** Used to get the current working dir when displaying the graphic. */
    DataModel& dataModel;

    /** Used to get the currently selected list item. */
    const LibraryWindow& libraryWindow;

    /** The active graphic set's ID. */
    EntityGraphicSetID activeGraphicSetID;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Text topText;
    AUI::Text modifyText;
    AUI::Text clearText;

    /** Holds this graphic set's graphics. */
    AUI::VerticalGridContainer graphicContainer;

    AUI::Text descText;
};

} // End namespace ResourceImporter
} // End namespace AM
