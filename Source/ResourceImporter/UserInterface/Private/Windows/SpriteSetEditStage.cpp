#include "SpriteSetEditStage.h"
#include "MainScreen.h"
#include "DataModel.h"
#include "NullSpriteID.h"
#include "LibraryItemData.h"
#include "Paths.h"
#include "AUI/Core.h"

namespace AM
{
namespace ResourceImporter
{
SpriteSetEditStage::SpriteSetEditStage(DataModel& inDataModel,
                               const LibraryWindow& inLibraryWindow)
: AUI::Window({320, 58, 1297, 1022}, "SpriteSetEditStage")
, dataModel{inDataModel}
, libraryWindow{inLibraryWindow}
, activeSpriteSetType{SpriteSet::Type::None}
, activeSpriteSetID{SDL_MAX_UINT16}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, modifyText{{277, 58, 800, 24}, "ModifyText"}
, clearText{{277, 88, 800, 24}, "ClearText"}
, spriteContainer{{559, 180, 180, 255}, "SpriteContainer"}
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
    children.push_back(spriteContainer);
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
                       "press one of the ASSIGN buttons.");
    styleText(clearText);
    clearText.setText("To clear: click an empty area to clear your selection, "
                      "then press one of the ASSIGN buttons.");
    styleText(descText1);
    styleText(descText2);
    styleText(descText3);
    styleText(descText4);
    styleText(descText5);

    /* Container */
    spriteContainer.setNumColumns(4);
    spriteContainer.setCellWidth(180);
    spriteContainer.setCellHeight(255 + 14);

    // When the active sprite set is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&SpriteSetEditStage::onActiveLibraryItemChanged>(*this);
    dataModel.spriteSetModel.spriteSetRemoved
        .connect<&SpriteSetEditStage::onSpriteSetRemoved>(*this);
    dataModel.spriteSetModel.spriteSetSlotChanged
        .connect<&SpriteSetEditStage::onSpriteSetSlotChanged>(*this);
}

void SpriteSetEditStage::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    if (std::holds_alternative<EditorFloorSpriteSet>(newActiveItem)) {
        loadActiveSpriteSet(SpriteSet::Type::Floor,
                            std::get<EditorFloorSpriteSet>(newActiveItem));
    }
    else if (std::holds_alternative<EditorFloorCoveringSpriteSet>(
                 newActiveItem)) {
        loadActiveSpriteSet(
            SpriteSet::Type::FloorCovering,
            std::get<EditorFloorCoveringSpriteSet>(newActiveItem));
    }
    else if (std::holds_alternative<EditorWallSpriteSet>(newActiveItem)) {
        loadActiveSpriteSet(SpriteSet::Type::Wall,
                            std::get<EditorWallSpriteSet>(newActiveItem));
    }
    else if (std::holds_alternative<EditorObjectSpriteSet>(newActiveItem)) {
        loadActiveSpriteSet(SpriteSet::Type::Object,
                            std::get<EditorObjectSpriteSet>(newActiveItem));
    }
    else {
        // New active item is not a sprite set. Clear this stage.
        activeSpriteSetType = SpriteSet::Type::None;
        activeSpriteSetID = SDL_MAX_UINT16;
        spriteContainer.setIsVisible(false);
    }
}

void SpriteSetEditStage::onSpriteSetRemoved(SpriteSet::Type type, Uint16 spriteSetID)
{
    if ((type == activeSpriteSetType) && (spriteSetID == activeSpriteSetID)) {
        activeSpriteSetType = SpriteSet::Type::None;
        activeSpriteSetID = SDL_MAX_UINT16;
        spriteContainer.setIsVisible(false);
    }
}

void SpriteSetEditStage::onSpriteSetSlotChanged(SpriteSet::Type type,
                                            Uint16 spriteSetID,
                                            std::size_t index, int newSpriteID)
{
    // If the changed data doesn't affect us, return early.
    if ((type != activeSpriteSetType) || (spriteSetID != activeSpriteSetID)) {
        return;
    }
    if (index > spriteContainer.size()) {
        LOG_FATAL("Index out of bounds.");
    }

    // Fill in the new slot data.
    SpriteSetSlot& slot{static_cast<SpriteSetSlot&>(*spriteContainer[index])};
    fillSlotSpriteData(slot, newSpriteID);
}

template<typename T>
void SpriteSetEditStage::loadActiveSpriteSet(SpriteSet::Type spriteSetType,
                                         const T& newActiveSpriteSet)
{
    activeSpriteSetType = spriteSetType;
    activeSpriteSetID = newActiveSpriteSet.numericID;

    // Fill the container with the sprite set's sprites.
    spriteContainer.clear();
    for (std::size_t i = 0; i < newActiveSpriteSet.spriteIDs.size(); ++i) {
        int spriteID{newActiveSpriteSet.spriteIDs[i]};

        // Construct the new slot widget.
        std::unique_ptr<AUI::Widget> slotPtr{std::make_unique<SpriteSetSlot>()};
        SpriteSetSlot& slot{static_cast<SpriteSetSlot&>(*slotPtr)};

        // Set the top text.
        slot.topText.setText(getSlotTopText(i));

        // Fill in the sprite's data.
        fillSlotSpriteData(slot, spriteID);

        // Set the assignment button callback.
        slot.assignButton.setOnPressed([this, &slot, i]() {
            // If a sprite is selected, set this slot to it.
            const auto& selectedListItems{libraryWindow.getSelectedListItems()};
            if (selectedListItems.size() > 0) {
                for (const LibraryListItem* selectedItem : selectedListItems) {
                    // If this is a sprite, update this slot in the model.
                    if (selectedItem->type == LibraryListItem::Type::Sprite) {
                        dataModel.spriteSetModel.setSpriteSetSlot(
                            activeSpriteSetType, activeSpriteSetID, i,
                            static_cast<Uint16>(selectedItem->ID));
                    }
                }
            }
            else {
                // No selection. Empty the slot.
                dataModel.spriteSetModel.setSpriteSetSlot(
                    activeSpriteSetType, activeSpriteSetID, i, NULL_SPRITE_ID);
            }
        });

        spriteContainer.push_back(std::move(slotPtr));
    }
    
    // Move and resize the container for the new type.
    if (activeSpriteSetType == SpriteSet::Type::Floor) {
        spriteContainer.setLogicalExtent({559, 180, 180, 255});
    }
    else {
        spriteContainer.setLogicalExtent({280, 180, 720, 524});
    }

    // Make sure the container is visible.
    spriteContainer.setIsVisible(true);

    // Fill in the appropriate description texts for the active sprite set type.
    fillDescriptionTexts();
}

void SpriteSetEditStage::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

std::string SpriteSetEditStage::getSlotTopText(std::size_t spriteSetIndex)
{
    switch (activeSpriteSetType) {
        case SpriteSet::Type::Floor: {
            // Floor slots have no top text.
            return "";
        }
        case SpriteSet::Type::FloorCovering:
        case SpriteSet::Type::Object: {
            switch (spriteSetIndex) {
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
        case SpriteSet::Type::Wall: {
            switch (spriteSetIndex) {
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

void SpriteSetEditStage::fillSlotSpriteData(SpriteSetSlot& slot, int spriteID)
{
    // If this slot isn't empty, set the widget's data.
    if (spriteID != NULL_SPRITE_ID) {
        // Calc a square texture extent that shows the bottom of the sprite
        // (so we don't have to squash it).
        const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};
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

void SpriteSetEditStage::fillDescriptionTexts()
{
    switch (activeSpriteSetType) {
        case SpriteSet::Type::Floor: {
            topText.setText("Floor Sprite Set");
            descText1.setText(
                "Floors are the lowest layer on a tile. They're rendered "
                "below everything else.");
            descText2.setText(
                "Floors have no collision, regardless of their sprite's "
                "collisionEnabled.");
            descText3.setText("There's no need to draw a bounding box on floor "
                              "sprites. Their tile "
                              "will be used for mouse hit detection.");
            descText4.setText("");
            descText5.setText("");
            break;
        }
        case SpriteSet::Type::FloorCovering: {
            topText.setText("Floor Covering Sprite Set");
            descText1.setText(
                "Floor coverings are things like rugs and puddles. They're "
                "rendered above floors, and below everything else.");
            descText2.setText("Each index is associated with a direction (in "
                              "parenthesis). You can ignore it if it isn't "
                              "applicable to your set of sprites.");
            descText3.setText("Floor coverings have no collision, regardless "
                              "of each sprite's collisionEnabled.");
            descText4.setText("Make sure to draw appropriate bounding boxes on "
                              "each sprite, as they will be used when clicking "
                              "the floor covering in build mode.");
            descText5.setText("At least 1 sprite must be set, but you don't "
                              "need to set sprites for every index. "
                              "Missing indices will be skipped.");
            break;
        }
        case SpriteSet::Type::Wall: {
            topText.setText("Wall Sprite Set");
            descText1.setText(
                "With these 4 wall sprite types, the engine's modular wall "
                "system is able to form any shape of wall.");
            descText2.setText("Walls may have collision. You can control this "
                              "using each sprite's collisionEnabled.");
            descText3.setText(
                "Make sure to draw appropriate bounding boxes on each sprite, "
                "as they will be used when clicking the object in build mode.");
            descText4.setText("All 4 sprites must be set.");
            descText5.setText("");
            break;
        }
        case SpriteSet::Type::Object: {
            topText.setText("Object Sprite Set");
            descText1.setText("Objects are anything that doesn't fit into the "
                              "other categories. Their render order is based "
                              "on their bounding box.");
            descText2.setText("Each index is associated with a direction (in "
                              "parenthesis). You can ignore it if it isn't "
                              "applicable to your set of sprites.");
            descText3.setText("Objects may have collision. You can control "
                              "this using each sprite's collisionEnabled.");
            descText4.setText(
                "Make sure to draw appropriate bounding boxes on each sprite, "
                "as they will be used when clicking the object in build mode.");
            descText5.setText("At least 1 sprite must be set, but you don't "
                              "need to set sprites for every index. "
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
