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
, graphicContainer{{559, 180, 180, 255}, "GraphicContainer"}
, descText1{{24, 806, 1240, 24}, "DescText1"}
, descText2{{24, 846, 1240, 24}, "DescText2"}
, descText3{{24, 886, 1240, 24}, "DescText3"}
, descText4{{24, 926, 1240, 24}, "DescText4"}
, descText5{{24, 966, 1240, 24}, "DescText5"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(modifyText);
    children.push_back(clearText);
    children.push_back(graphicContainer);
    children.push_back(descText1);
    children.push_back(descText2);
    children.push_back(descText3);
    children.push_back(descText4);
    children.push_back(descText5);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);

    styleText(modifyText);
    modifyText.setText("To modify: select a sprite in the Library window, then "
                       "press one of the Assign buttons.");
    styleText(clearText);
    clearText.setText("To clear: click an empty area to clear your selection, "
                      "then press one of the Assign buttons.");
    styleText(descText1);
    styleText(descText2);
    styleText(descText3);
    styleText(descText4);
    styleText(descText5);

    /* Container */
    graphicContainer.setNumColumns(4);
    graphicContainer.setCellWidth(180);
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
    if (holds_alternative<EditorFloorGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::Floor,
                             get<EditorFloorGraphicSet>(newActiveItem));
    }
    else if (holds_alternative<EditorFloorCoveringGraphicSet>(newActiveItem)) {
        loadActiveGraphicSet(GraphicSet::Type::FloorCovering,
                             get<EditorFloorCoveringGraphicSet>(newActiveItem));
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
        std::unique_ptr<AUI::Widget> slotPtr{std::make_unique<GraphicSetSlot>()};
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
    // If a graphic is selected, set this slot to it.
    const auto& selectedListItems{libraryWindow.getSelectedListItems()};
    if (selectedListItems.size() > 0) {
        for (const LibraryListItem* selectedItem : selectedListItems) {
            // If this is a sprite or animation, update this slot in 
            // the model.
            if (selectedItem->type == LibraryListItem::Type::Sprite) {
                SpriteID spriteID{static_cast<SpriteID>(selectedItem->ID)};
                dataModel.graphicSetModel.setGraphicSetSlot(
                    activeGraphicSetType, activeGraphicSetID, slotIndex,
                    toGraphicID(spriteID));
            }
            // TODO: Add animation support
            //else if (selectedItem->type == LibraryListItem::Type::Animation) {
            //}
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
        case GraphicSet::Type::Floor: {
            // Floor slots have no top text.
            return "";
        }
        case GraphicSet::Type::FloorCovering:
        case GraphicSet::Type::Object: {
            switch (graphicSetIndex) {
                case Rotation::Direction::South: {
                    return "0 (S)";
                }
                case Rotation::Direction::SouthWest: {
                    return "1 (SW)";
                }
                case Rotation::Direction::West: {
                    return "2 (W)";
                }
                case Rotation::Direction::NorthWest: {
                    return "3 (NW)";
                }
                case Rotation::Direction::North: {
                    return "4 (N)";
                }
                case Rotation::Direction::NorthEast: {
                    return "5 (NE)";
                }
                case Rotation::Direction::East: {
                    return "6 (E)";
                }
                case Rotation::Direction::SouthEast: {
                    return "7 (SE)";
                }
                default: {
                    return "";
                }
            }
        }
        case GraphicSet::Type::Wall: {
            switch (graphicSetIndex) {
                case Wall::Type::West: {
                    return "West";
                }
                case Wall::Type::North: {
                    return "North";
                }
                case Wall::Type::NorthWestGapFill: {
                    return "NW Gap Fill";
                }
                case Wall::Type::NorthEastGapFill: {
                    return "NE Gap Fill";
                }
                default: {
                    return "";
                }
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
        // Get the graphic's first sprite.
        EditorGraphicRef graphic{dataModel.getGraphic(graphicID)};
        const EditorSprite& sprite{graphic.getFirstSprite()};

        // Calc a square texture extent that shows the bottom of the sprite
        // (so we don't have to squash it).
        SDL_Rect textureExtent{sprite.textureExtent};
        if (textureExtent.h > textureExtent.w) {
            int diff{textureExtent.h - textureExtent.w};
            textureExtent.h -= diff;
            textureExtent.y += diff;
        }

        // Load the sprite's image into the slot.
        std::string imagePath{dataModel.getWorkingTexturesDir()};
        imagePath += sprite.parentSpriteSheetPath;
        slot.spriteImage.setSimpleImage(imagePath, textureExtent);
        slot.spriteImage.setIsVisible(true);

        // Set the text.
        slot.spriteNameText.setText(sprite.displayName);
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
        case GraphicSet::Type::Floor: {
            topText.setText("Floor Graphic Set");
            descText1.setText(
                "Floors are the lowest layer on a tile. They're rendered "
                "below everything else.");
            descText2.setText(
                "Floors have no collision, regardless of their graphics's "
                "collisionEnabled.");
            descText3.setText("There's no need to draw a bounding box on floor "
                              "graphics. Their tile "
                              "will be used for mouse hit detection.");
            descText4.setText("");
            descText5.setText("");
            break;
        }
        case GraphicSet::Type::FloorCovering: {
            topText.setText("Floor Covering Graphic Set");
            descText1.setText(
                "Floor coverings are things like rugs and puddles. They're "
                "rendered above floors, and below everything else.");
            descText2.setText("Each index is associated with a direction (in "
                              "parenthesis). You can ignore it if it isn't "
                              "applicable to your set of graphics.");
            descText3.setText("Floor coverings have no collision, regardless "
                              "of each graphic's collisionEnabled.");
            descText4.setText("Make sure to draw appropriate bounding boxes on "
                              "each graphic, as they will be used when clicking "
                              "the floor covering in build mode.");
            descText5.setText("At least 1 graphic must be set, but you don't "
                              "need to set graphics for every index. "
                              "Missing indices will be skipped.");
            break;
        }
        case GraphicSet::Type::Wall: {
            topText.setText("Wall Graphic Set");
            descText1.setText(
                "With these 4 wall graphic types, the engine's modular wall "
                "system is able to form any shape of wall.");
            descText2.setText("Walls may have collision. You can control this "
                              "using each graphic's collisionEnabled.");
            descText3.setText(
                "Make sure to draw appropriate bounding boxes on each graphic, "
                "as they will be used when clicking the object in build mode.");
            descText4.setText("All 4 graphic must be set.");
            descText5.setText("");
            break;
        }
        case GraphicSet::Type::Object: {
            topText.setText("Object Graphic Set");
            descText1.setText("Objects are anything that doesn't fit into the "
                              "other categories. Their render order is based "
                              "on their bounding box.");
            descText2.setText("Each index is associated with a direction (in "
                              "parenthesis). You can ignore it if it isn't "
                              "applicable to your set of graphics.");
            descText3.setText("Objects may have collision. You can control "
                              "this using each graphic's collisionEnabled.");
            descText4.setText(
                "Make sure to draw appropriate bounding boxes on each graphic, "
                "as they will be used when clicking the object in build mode.");
            descText5.setText("At least 1 graphic must be set, but you don't "
                              "need to set graphic for every index. "
                              "Missing indices will be skipped.");
            break;
        }
        default: {
            break;
        }
    }
}

} // End namespace ResourceImporter
} // End namespace AM
