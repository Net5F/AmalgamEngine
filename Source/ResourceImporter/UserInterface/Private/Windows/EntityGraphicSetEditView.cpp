#include "EntityGraphicSetEditView.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "SpriteID.h"
#include "LibraryItemData.h"
#include "Paths.h"
#include "DisplayStrings.h"
#include "AUI/Core.h"

namespace AM
{
namespace ResourceImporter
{
/** The width of a graphic container slot. */
static constexpr unsigned int SLOT_WIDTH{156};

EntityGraphicSetEditView::EntityGraphicSetEditView(
    DataModel& inDataModel,
                                       const LibraryWindow& inLibraryWindow)
: AUI::Window({320, 58, 1297, 1022}, "EntityGraphicSetEditView")
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeGraphicSetID{SDL_MAX_UINT16}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, modifyText{{193, 58, 1000, 24}, "ModifyText"}
, clearText{{193, 89, 1000, 24}, "ClearText"}
, graphicContainer{{25, 135, 1248, 632}, "GraphicContainer"}
, descText{{24, 807, 1240, 156}, "DescText"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(modifyText);
    children.push_back(clearText);
    children.push_back(graphicContainer);
    children.push_back(descText);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Entity Graphic Set");

    styleText(modifyText);
    modifyText.setText("To modify: select a sprite or animation in the Library window, then "
                       "press one of the Assign buttons.");
    styleText(clearText);
    clearText.setText("To clear: click an empty area to clear your selection, "
                      "then press one of the Assign buttons.");
    styleText(descText);
    descText.setText(
        "Entity Graphic Sets determine which graphics are shown when an entity "
        "enters its various states.\n\nThe Idle S graphic is required, but the "
        "rest are optional. If an entity enters a state that it doesn't have a "
        "graphic for, Idle S will be used.\n\nThe entity's bounding box will "
        "be determined by its Idle S graphic. Regardless of the graphic "
        "displayed, its bounding box will not change."); 

    /* Container */
    graphicContainer.setNumColumns(8);
    graphicContainer.setCellWidth(SLOT_WIDTH);
    graphicContainer.setCellHeight(255 + 14);
    initGraphicContainer();

    // When the active graphic set is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&EntityGraphicSetEditView::onActiveLibraryItemChanged>(*this);
    dataModel.entityGraphicSetModel.entityRemoved
        .connect<&EntityGraphicSetEditView::onEntityRemoved>(*this);
    dataModel.entityGraphicSetModel.entitySlotChanged
        .connect<&EntityGraphicSetEditView::onEntitySlotChanged>(*this);
}

void EntityGraphicSetEditView::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is an entity graphic set and return early if 
    // not.
    const EditorEntityGraphicSet* newActiveGraphicSet{
        get_if<EditorEntityGraphicSet>(&newActiveItem)};
    if (!newActiveGraphicSet) {
        activeGraphicSetID = SDL_MAX_UINT16;
        graphicContainer.setIsVisible(false);
        return;
    }

    activeGraphicSetID = newActiveGraphicSet->numericID;

    // Fill the container with the graphic set's graphics.
    for (auto& [graphicType, graphicIDArr] : newActiveGraphicSet->graphicIDs) {
        for (Uint8 i{0}; i < Rotation::Direction::Count; ++i) {
            GraphicID graphicID{graphicIDArr.at(i)};

            // Get the graphic set slot widget that matches graphicType.
            std::size_t index{
                toIndex(graphicType, static_cast<Rotation::Direction>(i))};
            GraphicSetSlot& slot{
                static_cast<GraphicSetSlot&>(*graphicContainer[index])};

            // Fill in the graphic's data.
            fillSlotGraphicData(slot, graphicID);
        }
    }

    // Make sure the container is visible.
    graphicContainer.setIsVisible(true);
}

void EntityGraphicSetEditView::onEntityRemoved(EntityGraphicSetID graphicSetID)
{
    // If the active entity set was deleted, hide this window.
    if (graphicSetID == activeGraphicSetID) {
        activeGraphicSetID = SDL_MAX_UINT16;
        setIsVisible(false);
    }
}

void EntityGraphicSetEditView::onEntitySlotChanged(
    EntityGraphicSetID graphicSetID, EntityGraphicType graphicType,
    Rotation::Direction direction, GraphicID newGraphicID)
{
    // If the changed data doesn't affect us, return early.
    if (graphicSetID != activeGraphicSetID) {
        return;
    }

    // Fill in the new slot data.
    std::size_t index{toIndex(graphicType, direction)};
    GraphicSetSlot& slot{
        static_cast<GraphicSetSlot&>(*graphicContainer[index])};
    fillSlotGraphicData(slot, newGraphicID);
}

void EntityGraphicSetEditView::onAssignButtonPressed(
    EntityGraphicType graphicType, Rotation::Direction direction)
{
    // If a graphic is selected, set the given slot to it.
    // Note: This just uses the first selected graphic. Multi-select is ignored.
    const auto& selectedListItems{libraryWindow.getSelectedListItems()};
    if (selectedListItems.size() > 0) {
        for (const LibraryListItem* selectedItem : selectedListItems) {
            // If this is a sprite or animation, update the given model slot.
            if (selectedItem->type == LibraryListItem::Type::Sprite) {
                SpriteID spriteID{static_cast<SpriteID>(selectedItem->ID)};
                dataModel.entityGraphicSetModel.setEntitySlot(
                    activeGraphicSetID, graphicType, direction,
                    toGraphicID(spriteID));
                break;
            }
            else if (selectedItem->type == LibraryListItem::Type::Animation) {
                AnimationID animationID{
                    static_cast<AnimationID>(selectedItem->ID)};
                dataModel.entityGraphicSetModel.setEntitySlot(
                    activeGraphicSetID, graphicType, direction,
                    toGraphicID(animationID));
                break;
            }
        }
    }
    else {
        // No selection. Empty the slot.
        dataModel.entityGraphicSetModel.setEntitySlot(
            activeGraphicSetID, graphicType, direction, NULL_GRAPHIC_ID);
    }
}

void EntityGraphicSetEditView::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

void EntityGraphicSetEditView::initGraphicContainer()
{
    // Fill the container with a slot widget for each EntityGraphicType 
    // and Rotation::Direction.
    graphicContainer.clear();
    iterateEntityGraphicTypes([&](EntityGraphicType graphicType) {
        for (Uint8 j{0}; j < Rotation::Direction::Count; j++) {
            Rotation::Direction direction{static_cast<Rotation::Direction>(j)};

            // Construct the new slot widget.
            std::unique_ptr<AUI::Widget> slotPtr{
                std::make_unique<GraphicSetSlot>(SLOT_WIDTH)};
            GraphicSetSlot& slot{static_cast<GraphicSetSlot&>(*slotPtr)};

            // Set the top text.
            std::string topText{DisplayStrings::get(graphicType)};
            topText += " " + DisplayStrings::get(direction);
            slot.topText.setText(topText);

            // Fill in the graphic's data.
            fillSlotGraphicData(slot, NULL_GRAPHIC_ID);

            // Set the assignment button callback.
            slot.assignButton.setOnPressed([this, graphicType, direction]() {
                onAssignButtonPressed(graphicType, direction);
            });

            graphicContainer.push_back(std::move(slotPtr));
        }
    });
}

void EntityGraphicSetEditView::fillSlotGraphicData(GraphicSetSlot& slot,
                                                   GraphicID graphicID)
{
    // If this slot isn't empty, set the widget's data.
    if (graphicID) {
        // Set the text.
        EditorGraphicRef graphic{dataModel.getGraphic(graphicID)};
        slot.spriteNameText.setText(graphic.getDisplayName());

        // Get the graphic's first sprite.
        const EditorSprite* sprite{graphic.getFirstSprite()};
        if (!sprite) {
            // Graphic doesn't have a sprite (empty animation). Leave the 
            // image blank.
            slot.spriteImage.setIsVisible(false);
            return;
        }

        // Calc a square texture extent that shows the bottom of the sprite
        // (so we don't have to squash it).
        SDL_Rect textureExtent{sprite->textureExtent};
        textureExtent.x = 0;
        textureExtent.y = 0;
        if (textureExtent.h > textureExtent.w) {
            int diff{textureExtent.h - textureExtent.w};
            textureExtent.h -= diff;
            textureExtent.y += diff;
        }

        // Load the sprite's image into the slot.
        std::string imagePath{dataModel.getWorkingIndividualSpritesDir()};
        imagePath += sprite->imagePath;
        slot.spriteImage.setSimpleImage(imagePath, textureExtent);
        slot.spriteImage.setIsVisible(true);
    }
    else {
        // Empty slot.
        slot.spriteImage.setIsVisible(false);
        slot.spriteNameText.setText("Empty");
    }
}

std::size_t EntityGraphicSetEditView::toIndex(EntityGraphicType graphicType,
                                              Rotation::Direction direction)
{
    AM_ASSERT(graphicType != EntityGraphicType::NotSet,
              "Tried to get index of uninitialized entity graphic type.");

    return ((static_cast<std::size_t>(graphicType) - 1)
            * Rotation::Direction::Count)
           + direction;
}

template<typename Func>
void EntityGraphicSetEditView::iterateEntityGraphicTypes(Func callback)
{
    static constexpr std::size_t NotSet{
        static_cast<std::size_t>(EntityGraphicType::NotSet)};
    static constexpr std::size_t PROJECT_START{
        static_cast<std::size_t>(EntityGraphicType::PROJECT_START)};
    static constexpr std::size_t PROJECT_END{
        static_cast<std::size_t>(EntityGraphicType::PROJECT_END)};

    // Engine types
    for (std::size_t i{NotSet + 1}; i < PROJECT_START; ++i) {
        callback(static_cast<EntityGraphicType>(i));
    }

    // Project types
    for (std::size_t i{PROJECT_START + 1}; i < PROJECT_END; ++i) {
        callback(static_cast<EntityGraphicType>(i));
    }
}

} // End namespace ResourceImporter
} // End namespace AM
