#pragma once

#include "LibraryListItem.h"
#include "SpriteSheetID.h"
#include "SpriteID.h"
#include "BoundingBoxID.h"
#include "AnimationID.h"
#include "GraphicSets.h"
#include "IconID.h"
#include "AUI/Window.h"
#include "AUI/Text.h"
#include "AUI/Image.h"
#include "AUI/VerticalListContainer.h"
#include "AUI/Button.h"
#include "entt/signal/sigh.hpp"
#include <unordered_map>

namespace AM
{
namespace ResourceImporter
{
class MainScreen;
class DataModel;
struct EditorSpriteSheet;
struct EditorSprite;
struct EditorAnimation;
struct EditorBoundingBox;
class ParentListItem;
struct EditorTerrainGraphicSet;
struct EditorFloorGraphicSet;
struct EditorWallGraphicSet;
struct EditorObjectGraphicSet;
struct EditorEntityGraphicSet;
struct EditorIconSheet;

/**
 * The left-side panel on the main screen. Allows the user to manage the
 * project's sprite sheets, sprite sets, etc.
 */
class LibraryWindow : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    LibraryWindow(MainScreen& inScreen, DataModel& inDataModel);

    /**
     * Returns the currently selected list items.
     */
    const std::vector<LibraryListItem*>& getSelectedListItems() const;

    //-------------------------------------------------------------------------
    // Base class overrides
    //-------------------------------------------------------------------------
    void onFocusLost(AUI::FocusLostType focusLostType) override;

    AUI::EventResult onKeyDown(SDL_Keycode keyCode) override;

private:
    /**
     * The top-level categories that we have in the library.
     * These values are used to index into libraryContainer.
     */
    enum Category {
        SpriteSheets,
        Animations,
        BoundingBoxes,
        Terrain,
        Floors,
        Walls,
        Objects,
        Entities,
        IconSheets,
        Count,
        None
    };

    /**
     * Adds the given item to the library.
     */
    void onSpriteSheetAdded(SpriteSheetID sheetID,
                            const EditorSpriteSheet& sheet);
    void onSpriteAdded(SpriteID spriteID, const EditorSprite& sprite,
                       SpriteSheetID parentSheetID);
    void onAnimationAdded(AnimationID animationID,
                          const EditorAnimation& animation);
    void onBoundingBoxAdded(BoundingBoxID boundingBoxID,
                            const EditorBoundingBox& bounds);
    void onTerrainAdded(TerrainGraphicSetID terrainID,
                        const EditorTerrainGraphicSet& terrain);
    void onFloorAdded(FloorGraphicSetID floorID,
                      const EditorFloorGraphicSet& floor);
    void onWallAdded(WallGraphicSetID wallID, const EditorWallGraphicSet& wall);
    void onObjectAdded(ObjectGraphicSetID objectID,
                       const EditorObjectGraphicSet& object);
    template<typename T>
    void onGraphicSetAdded(Uint16 graphicSetID, const T& graphicSet);
    void onEntityAdded(EntityGraphicSetID graphicSetID,
                       const EditorEntityGraphicSet& entity);
    void onIconSheetAdded(int sheetID, const EditorIconSheet& sheet);

    /**
     * Removes the given item from the library.
     */
    void onSpriteRemoved(SpriteID spriteID, SpriteSheetID parentSheetID);
    void onSpriteSheetRemoved(SpriteSheetID sheetID);
    void onAnimationRemoved(AnimationID animationID);
    void onBoundingBoxRemoved(BoundingBoxID boundingBoxID);
    void onGraphicSetRemoved(GraphicSet::Type type, Uint16 graphicSetID);
    void onEntityRemoved(EntityGraphicSetID graphicSetID);
    void onIconSheetRemoved(int sheetID);

    /**
     * Updates the display name on the associated list item.
     */
    void onSpriteSheetDisplayNameChanged(SpriteSheetID spriteSheetID,
                                         const std::string& newDisplayName);
    void onSpriteDisplayNameChanged(SpriteID spriteID,
                                    const std::string& newDisplayName);
    void onAnimationDisplayNameChanged(AnimationID animationID,
                                       const std::string& newDisplayName);
    void onBoundingBoxDisplayNameChanged(BoundingBoxID boundingBoxID,
                                         const std::string& newDisplayName);
    void onGraphicSetDisplayNameChanged(GraphicSet::Type type, Uint16 graphicSetID,
                                       const std::string& newDisplayName);
    void onEntityDisplayNameChanged(EntityGraphicSetID graphicSetID,
                                    const std::string& newDisplayName);
    void onIconDisplayNameChanged(IconID iconID,
                                  const std::string& newDisplayName);

    /**
     * Adds the given icon to the given icon sheet list item.
     */
    void addIconToSheetListItem(ParentListItem& sheetListItem,
                                const EditorIconSheet& sheet, IconID iconID);

    /**
     * If there are other currently selected list items, checks if the given
     * list item is compatible with them. If so, adds it to the vector.
     */
    void processSelectedListItem(LibraryListItem* selectedListItem);

    /**
     * If the given list item is selected, removes it from the vector.
     */
    void processDeselectedListItem(LibraryListItem* deselectedListItem);

    /**
     * Removes the given list item widget from the library and all secondary
     * data structures.
     */
    void removeListItem(LibraryListItem* listItem);

    LibraryListItem::Type toListItemType(GraphicSet::Type graphicSetType);
    Category toCategory(GraphicSet::Type graphicSetType);

    /** Used to open the confirmation dialog when removing a sheet. */
    MainScreen& mainScreen;

    /** Used to update the model when a sheet is removed. */
    DataModel& dataModel;

    /** An array of maps, indexed by list item types. Each map holds the
        list items for the associated type, with the associated IDs as keys. */
    std::array<std::unordered_map<int, LibraryListItem*>,
               LibraryListItem::Type::Count>
        listItemMaps;

    /** Holds the currently selected list items. */
    std::vector<LibraryListItem*> selectedListItems;

    /** Holds items that are staged to be removed. */
    std::vector<LibraryListItem*> itemsToRemove;

    //-------------------------------------------------------------------------
    // Private child widgets
    //-------------------------------------------------------------------------
    AUI::Image backgroundImage;

    AUI::Image headerImage;

    AUI::Text windowLabel;

    AUI::VerticalListContainer libraryContainer;

    AUI::Button addButton;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(const std::vector<LibraryListItem*>& selectedItems)>
        selectedItemsChangedSig;

public:
    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** The list of selected library items has changed. */
    entt::sink<
        entt::sigh<void(const std::vector<LibraryListItem*>& selectedItems)>>
        selectedItemsChanged;
};

} // End namespace ResourceImporter
} // End namespace AM
