#include "GraphicSetEditStage.h"
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
GraphicSetEditStage::GraphicSetEditStage(DataModel& inDataModel,
                                       const LibraryWindow& inLibraryWindow)
: AUI::Window({320, 58, 1297, 1022}, "GraphicSetEditStage")
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeGraphicSetType{GraphicSet::Type::None}
, activeGraphicSetID{SDL_MAX_UINT16}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, modifyText{{277, 58, 800, 24}, "ModifyText"}
, clearText{{277, 88, 800, 24}, "ClearText"}
, graphicContainer{{249, 180, 180, 255}, "GraphicContainer"}
, descText{{24, 806, 1240, 156}, "DescText"}
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

    styleText(modifyText);
    modifyText.setText("To modify: select a sprite or animation in the Library window, then "
                       "press one of the Assign buttons.");
    styleText(clearText);
    clearText.setText("To clear: click an empty area to clear your selection, "
                      "then press one of the Assign buttons.");
    styleText(descText);

    /* Container */
    graphicContainer.setNumColumns(4);
    graphicContainer.setCellWidth(200);
    graphicContainer.setCellHeight(255 + 14);

    // When the active graphic set is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&GraphicSetEditStage::onActiveLibraryItemChanged>(*this);
    dataModel.graphicSetModel.graphicSetRemoved
        .connect<&GraphicSetEditStage::onGraphicSetRemoved>(*this);
    dataModel.graphicSetModel.graphicSetSlotChanged
        .connect<&GraphicSetEditStage::onGraphicSetSlotChanged>(*this);
}

void GraphicSetEditStage::onActiveLibraryItemChanged(
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

void GraphicSetEditStage::onGraphicSetRemoved(GraphicSet::Type type,
                                            Uint16 graphicSetID)
{
    if ((type == activeGraphicSetType) && (graphicSetID == activeGraphicSetID)) {
        activeGraphicSetType = GraphicSet::Type::None;
        activeGraphicSetID = SDL_MAX_UINT16;
        graphicContainer.setIsVisible(false);
    }
}

void GraphicSetEditStage::onGraphicSetSlotChanged(GraphicSet::Type type,
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
void GraphicSetEditStage::loadActiveGraphicSet(GraphicSet::Type graphicSetType,
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

    // Move and resize the container for the new type.
    if (activeGraphicSetType == GraphicSet::Type::Floor) {
        graphicContainer.setLogicalExtent({559, 180, 180, 255});
    }
    else {
        graphicContainer.setLogicalExtent({280, 180, 720, 524});
    }

    // Make sure the container is visible.
    graphicContainer.setIsVisible(true);

    // Fill in the appropriate description texts for the active graphic set type.
    fillDescriptionTexts();
}

void GraphicSetEditStage::onAssignButtonPressed(std::size_t slotIndex)
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

void GraphicSetEditStage::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

std::string GraphicSetEditStage::getSlotTopText(std::size_t graphicSetIndex)
{
    switch (activeGraphicSetType) {
        case GraphicSet::Type::Terrain: {
            switch (graphicSetIndex) {
                case Terrain::Type::Flat:
                    return "Flat";
                case Terrain::Type::BlockOneQuarter:
                    return "Block 1/4";
                case Terrain::Type::BlockHalf:
                    return "Block 1/2";
                case Terrain::Type::BlockThreeQuarter:
                    return "Block 3/4";
                case Terrain::Type::BlockFull:
                    return "Block Full";
                case Terrain::Type::WedgeSOneQuarter:
                    return "Wedge S 1/4";
                case Terrain::Type::WedgeSHalf:
                    return "Wedge S 1/2";
                case Terrain::Type::WedgeSThreeQuarter:
                    return "Wedge S 3/4";
                case Terrain::Type::WedgeSFull:
                    return "Wedge S Full";
                case Terrain::Type::WedgeSWOneQuarter:
                    return "Wedge SW 1/4";
                case Terrain::Type::WedgeSWHalf:
                    return "Wedge SW 1/2";
                case Terrain::Type::WedgeSWThreeQuarter:
                    return "Wedge SW 3/4";
                case Terrain::Type::WedgeSWFull:
                    return "Wedge SW Full";
                case Terrain::Type::WedgeWOneQuarter:
                    return "Wedge W 1/4";
                case Terrain::Type::WedgeWHalf:
                    return "Wedge W 1/2";
                case Terrain::Type::WedgeWThreeQuarter:
                    return "Wedge W 3/4";
                case Terrain::Type::WedgeWFull:
                    return "Wedge W Full";
                case Terrain::Type::WedgeNWOneQuarter:
                    return "Wedge NW 1/4";
                case Terrain::Type::WedgeNWHalf:
                    return "Wedge NW 1/2";
                case Terrain::Type::WedgeNWThreeQuarter:
                    return "Wedge NW 3/4";
                case Terrain::Type::WedgeNWFull:
                    return "Wedge NW Full";
                case Terrain::Type::WedgeNOneQuarter:
                    return "Wedge N 1/4";
                case Terrain::Type::WedgeNHalf:
                    return "Wedge N 1/2";
                case Terrain::Type::WedgeNThreeQuarter:
                    return "Wedge N 3/4";
                case Terrain::Type::WedgeNFull:
                    return "Wedge N Full";
                case Terrain::Type::WedgeNEOneQuarter:
                    return "Wedge NE 1/4";
                case Terrain::Type::WedgeNEHalf:
                    return "Wedge NE 1/2";
                case Terrain::Type::WedgeNEThreeQuarter:
                    return "Wedge NE 3/4";
                case Terrain::Type::WedgeNEFull:
                    return "Wedge NE Full";
                case Terrain::Type::WedgeEOneQuarter:
                    return "Wedge E 1/4";
                case Terrain::Type::WedgeEHalf:
                    return "Wedge E 1/2";
                case Terrain::Type::WedgeEThreeQuarter:
                    return "Wedge E 3/4";
                case Terrain::Type::WedgeEFull:
                    return "Wedge E Full";
                case Terrain::Type::WedgeSEOneQuarter:
                    return "Wedge SE 1/4";
                case Terrain::Type::WedgeSEHalf:
                    return "Wedge SE 1/2";
                case Terrain::Type::WedgeSEThreeQuarter:
                    return "Wedge SE 3/4";
                case Terrain::Type::WedgeSEFull:
                    return "Wedge SE Full";
            }
        }
        case GraphicSet::Type::Floor:
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

void GraphicSetEditStage::fillSlotGraphicData(GraphicSetSlot& slot,
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
        std::string imagePath{dataModel.getWorkingTexturesDir()};
        imagePath += sprite->parentSpriteSheetPath;
        slot.spriteImage.setSimpleImage(imagePath, textureExtent);
        slot.spriteImage.setIsVisible(true);
    }
    else {
        // Empty slot.
        slot.spriteImage.setIsVisible(false);
        slot.spriteNameText.setText("Empty");
    }
}

void GraphicSetEditStage::fillDescriptionTexts()
{
    switch (activeGraphicSetType) {
        case GraphicSet::Type::Terrain: {
            topText.setText("Terrain Graphic Set");
            descText.setText(
                "Terrain is comprised of flat ground, blocks, and wedges. "
                "Blocks and wedges each have 4 height options, with �Full� "
                "height equaling the height of a world tile. Wedges can face "
                "any of the 8 directions.\n\nAll terrain graphics are "
                "optional. This allows developers to leave out graphics if "
                "they aren't going to use them, e.g. only implementing 1/2 and "
                "full heights and leaving out 1/4 and 3/4. It's up to the "
                "build tool to properly surface valid options.\n\nCollision "
                "geometry is auto-generated for each piece of terrain based on "
                "its type. You don't need to bother adding bounding geometry "
                "to terrain sprites. All terrain has collision, regardless of "
                "each sprite's collisionEnabled.");
            break;
        }
        case GraphicSet::Type::Floor: {
            topText.setText("Floor Graphic Set");
            descText.setText(
                "Floors are things like grass, rugs, flooring, etc.\n\nEach "
                "index is associated with a direction (in parenthesis). You "
                "can ignore it if it isn't applicable to your set of "
                "graphics.\n\nFloors have no collision, regardless of each "
                "graphic's collisionEnabled.\n\nMake sure to draw appropriate "
                "bounding boxes on each graphic, as they will be used when "
                "clicking the floor in build mode.\n\nYou don't need to set "
                "graphics for every index. Missing indices will be skipped by "
                "the build tool.");
            break;
        }
        case GraphicSet::Type::Wall: {
            topText.setText("Wall Graphic Set");
            descText.setText(
                "With these 4 wall graphic types, the engine's modular wall "
                "system is able to form any shape of wall.\n\nWalls may have "
                "collision. You can control this using each graphic's "
                "collisionEnabled.\n\nMake sure to draw appropriate bounding "
                "boxes on each graphic, as they will be used when clicking the "
                "object in build mode.\n\nAll 4 graphics must be set.");
            break;
        }
        case GraphicSet::Type::Object: {
            topText.setText("Object Graphic Set");
            descText.setText(
                "Objects are anything that doesn't fit into the other "
                "categories. Their render order is based on their bounding "
                "box.\n\nEach index is associated with a direction (in "
                "parenthesis). You can ignore it if it isn't applicable to "
                "your set of graphics.\n\nObjects may have collision. You can "
                "control this using each graphic's collisionEnabled.\n\nMake "
                "sure to draw appropriate bounding boxes on each graphic, as "
                "they will be used when clicking the object in build "
                "mode.\n\nYou don't need to set a graphic for every index. "
                "Missing indices will be skipped by the build tool.");
            break;
        }
        default: {
            break;
        }
    }
}

} // End namespace ResourceImporter
} // End namespace AM
