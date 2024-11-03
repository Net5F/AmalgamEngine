#include "GraphicSetEditView.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "SpriteID.h"
#include "LibraryItemData.h"
#include "Paths.h"
#include "AUI/Core.h"

namespace AM
{
namespace ResourceImporter
{
GraphicSetEditView::GraphicSetEditView(DataModel& inDataModel,
                                       const LibraryWindow& inLibraryWindow)
: AUI::Window({320, 58, 1297, 1022}, "GraphicSetEditView")
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeGraphicSetType{GraphicSet::Type::None}
, activeGraphicSetID{SDL_MAX_UINT16}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, actionText{{184, 58, 929, 50}, "ActionText"}
, graphicContainer{{249, 180, 800, 516}, "GraphicContainer"}
, descText{{24, 806, 1240, 220}, "DescText"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(actionText);
    children.push_back(graphicContainer);
    children.push_back(descText);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);

    styleText(actionText);
    actionText.setText(
        "To modify: select a sprite or animation in the Library window, then "
        "press one of the Assign buttons.\nTo clear: click an empty area to "
        "clear your selection, then press one of the Assign buttons.");
    styleText(descText);

    /* Container */
    graphicContainer.setNumColumns(4);
    graphicContainer.setCellWidth(200);
    graphicContainer.setCellHeight(247 + 22);

    // When the active graphic set is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&GraphicSetEditView::onActiveLibraryItemChanged>(*this);
    dataModel.graphicSetModel.graphicSetRemoved
        .connect<&GraphicSetEditView::onGraphicSetRemoved>(*this);
    dataModel.graphicSetModel.graphicSetSlotChanged
        .connect<&GraphicSetEditView::onGraphicSetSlotChanged>(*this);
}

void GraphicSetEditView::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    if (holds_alternative<EditorTerrainGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::Terrain,
                             get<EditorTerrainGraphicSet>(newActiveItem));
    }
    else if (holds_alternative<EditorFloorGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::Floor,
                             get<EditorFloorGraphicSet>(newActiveItem));
    }
    else if (holds_alternative<EditorWallGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::Wall,
                             get<EditorWallGraphicSet>(newActiveItem));
    }
    else if (holds_alternative<EditorObjectGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::Object,
                             get<EditorObjectGraphicSet>(newActiveItem));
    }
    else {
        // New active item is not a graphic set. Clear this stage.
        activeGraphicSetType = GraphicSet::Type::None;
        activeGraphicSetID = SDL_MAX_UINT16;
        graphicContainer.setIsVisible(false);
    }
}

void GraphicSetEditView::onGraphicSetRemoved(GraphicSet::Type type,
                                            Uint16 graphicSetID)
{
    if ((type == activeGraphicSetType) && (graphicSetID == activeGraphicSetID)) {
        activeGraphicSetType = GraphicSet::Type::None;
        activeGraphicSetID = SDL_MAX_UINT16;
        graphicContainer.setIsVisible(false);
    }
}

void GraphicSetEditView::onGraphicSetSlotChanged(GraphicSet::Type type,
                                                Uint16 graphicSetID,
                                                std::size_t index,
                                                GraphicID newGraphicID)
{
    // If the changed data doesn't affect us, return early.
    if ((type != activeGraphicSetType) || (graphicSetID != activeGraphicSetID)) {
        return;
    }
    if (index > graphicContainer.size()) {
        LOG_FATAL("Index out of bounds.");
    }

    // Fill in the new slot data.
    GraphicSetSlot& slot{static_cast<GraphicSetSlot&>(*graphicContainer[index])};
    fillSlotGraphicData(slot, newGraphicID);
}

template<typename T>
void GraphicSetEditView::loadActiveGraphicSet(GraphicSet::Type graphicSetType,
                                               const T& newActiveGraphicSet)
{
    activeGraphicSetType = graphicSetType;
    activeGraphicSetID = newActiveGraphicSet.numericID;

    // Fill the container with the graphic set's graphics.
    graphicContainer.clear();
    for (std::size_t i = 0; i < newActiveGraphicSet.graphicIDs.size(); ++i) {
        GraphicID graphicID{newActiveGraphicSet.graphicIDs[i]};

        // Construct the new slot widget.
        std::unique_ptr<AUI::Widget> slotPtr{
            std::make_unique<GraphicSetSlot>(200)};
        GraphicSetSlot& slot{static_cast<GraphicSetSlot&>(*slotPtr)};

        // Set the top text.
        slot.topText.setText(getSlotTopText(i));

        // Fill in the graphic's data.
        fillSlotGraphicData(slot, graphicID);

        // Set the assignment button callback.
        slot.assignButton.setOnPressed([this, i]() {
            onAssignButtonPressed(i);
        });

        graphicContainer.push_back(std::move(slotPtr));
    }

    // Make sure the container is visible.
    graphicContainer.setIsVisible(true);

    // Fill in the appropriate description texts for the active graphic set type.
    fillDescriptionTexts();
}

void GraphicSetEditView::onAssignButtonPressed(std::size_t slotIndex)
{
    // If a graphic is selected, set the given slot to it.
    // Note: This just uses the first selected graphic. Multi-select is ignored.
    const auto& selectedListItems{libraryWindow.getSelectedListItems()};
    if (selectedListItems.size() > 0) {
        for (const LibraryListItem* selectedItem : selectedListItems) {
            // If this is a sprite or animation, update the given model slot.
            if (selectedItem->type == LibraryListItem::Type::Sprite) {
                SpriteID spriteID{static_cast<SpriteID>(selectedItem->ID)};
                dataModel.graphicSetModel.setGraphicSetSlot(
                    activeGraphicSetType, activeGraphicSetID, slotIndex,
                    toGraphicID(spriteID));
                break;
            }
            else if (selectedItem->type == LibraryListItem::Type::Animation) {
                AnimationID animationID{
                    static_cast<AnimationID>(selectedItem->ID)};
                dataModel.graphicSetModel.setGraphicSetSlot(
                    activeGraphicSetType, activeGraphicSetID, slotIndex,
                    toGraphicID(animationID));
                break;
            }
        }
    }
    else {
        // No selection. Empty the slot.
        dataModel.graphicSetModel.setGraphicSetSlot(activeGraphicSetType,
                                                    activeGraphicSetID,
                                                    slotIndex, NULL_GRAPHIC_ID);
    }
}

void GraphicSetEditView::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

std::string GraphicSetEditView::getSlotTopText(std::size_t graphicSetIndex)
{
    switch (activeGraphicSetType) {
        case GraphicSet::Type::Terrain: {
            switch (graphicSetIndex) {
                case Terrain::Height::Flat:
                    return "Flat";
                case Terrain::Height::OneThird:
                    return "1/3 Height";
                case Terrain::Height::TwoThird:
                    return "2/3 Height";
                case Terrain::Height::Full:
                    return "Full Height";
            }
        }
        case GraphicSet::Type::Floor:
            [[fallthrough]];
        case GraphicSet::Type::Object: {
            switch (graphicSetIndex) {
                case Rotation::Direction::South:
                    return "0 (S)";
                case Rotation::Direction::SouthWest:
                    return "1 (SW)";
                case Rotation::Direction::West:
                    return "2 (W)";
                case Rotation::Direction::NorthWest:
                    return "3 (NW)";
                case Rotation::Direction::North:
                    return "4 (N)";
                case Rotation::Direction::NorthEast:
                    return "5 (NE)";
                case Rotation::Direction::East:
                    return "6 (E)";
                case Rotation::Direction::SouthEast:
                    return "7 (SE)";
                default:
                    return "";
            }
        }
        case GraphicSet::Type::Wall: {
            switch (graphicSetIndex) {
                case Wall::Type::West:
                    return "West";
                case Wall::Type::North:
                    return "North";
                case Wall::Type::NorthWestGapFill:
                    return "NW Gap Fill";
                case Wall::Type::NorthEastGapFill:
                    return "NE Gap Fill";
                default:
                    return "";
            }
        }
        default: {
            return "";
        }
    }

    return "";
}

void GraphicSetEditView::fillSlotGraphicData(GraphicSetSlot& slot,
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
        if (textureExtent.h > textureExtent.w) {
            int diff{textureExtent.h - textureExtent.w};
            textureExtent.h -= diff;
            textureExtent.y += diff;
        }

        // Load the sprite's image into the slot.
        std::string imagePath{dataModel.getWorkingIndividualSpritesDir()};
        imagePath += sprite->imagePath;
        slot.spriteImage.setSimpleImage(
            imagePath,
            {0, 0, sprite->textureExtent.w, sprite->textureExtent.h});
        slot.spriteImage.setIsVisible(true);
    }
    else {
        // Empty slot.
        slot.spriteImage.setIsVisible(false);
        slot.spriteNameText.setText("Empty");
    }
}

void GraphicSetEditView::fillDescriptionTexts()
{
    switch (activeGraphicSetType) {
        case GraphicSet::Type::Terrain: {
            topText.setText("Terrain Graphic Set");
            descText.setText(
                "Terrain is comprised of blocks of various heights. All block "
                "heights are relative to the height of a world-space tile "
                "(SharedConfig::TILE_WORLD_HEIGHT).\n\nAt least 1 graphic must "
                "be set, but you don't need to set every graphic. The build "
                "tool should skip missing graphics.\n\nCollision geometry is "
                "auto-generated for each piece of terrain based on its height. "
                "You don't need to bother adding bounding geometry to terrain "
                "graphic. All terrain has collision, regardless of each "
                "graphic's collisionEnabled.");
            break;
        }
        case GraphicSet::Type::Floor: {
            topText.setText("Floor Graphic Set");
            descText.setText(
                "Floors are things like grass, rugs, flooring, etc. They sit "
                "on top of the terrain and have no collision (regardless of "
                "each graphic's collisionEnabled).\n\nMake sure to draw "
                "appropriate bounding volumes on each floor graphic, as they "
                "will be used when clicking the floor in build mode.\n\nEach "
                "index is associated with a direction (in parenthesis). You "
                "can ignore it if it isn't applicable to your set of "
                "graphics.\nAt least 1 graphic must be set, but you don't need "
                "to set every graphic. The build tool should skip missing "
                "graphics.");
            break;
        }
        case GraphicSet::Type::Wall: {
            topText.setText("Wall Graphic Set");
            descText.setText(
                "With these 4 wall graphic types, the engine's modular wall "
                "system is able to form any shape of wall.\n\nWalls may have "
                "collision. You can control this using each graphic's "
                "collisionEnabled.\n\nMake sure to draw appropriate bounding "
                "volumes on each graphic, as they will be used when clicking the "
                "object in build mode.\n\nAll 4 graphics must be set.");
            break;
        }
        case GraphicSet::Type::Object: {
            topText.setText("Object Graphic Set");
            descText.setText(
                "Objects are anything that doesn't fit into the other "
                "categories.\n\nEach index is associated with a direction (in "
                "parenthesis). You can ignore it if it isn't applicable to "
                "your set of sprites.\n\nMake sure to draw appropriate "
                "bounding volumes on each graphic, as they will be used when "
                "clicking the object in build mode.\n\nObjects may have "
                "collision. You can control this using each graphic's "
                "collisionEnabled.\n\nAt least 1 graphic must be set, but you "
                "don't need to set graphics for every index. The build tool "
                "should skip missing graphics.");
            break;
        }
        default: {
            break;
        }
    }
}

} // End namespace ResourceImporter
} // End namespace AM
