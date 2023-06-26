#include "FloorEditStage.h"
#include "MainScreen.h"
#include "EditorFloorSpriteSet.h"
#include "EmptySpriteID.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "Ignore.h"
#include "AUI/Core.h"

namespace AM
{
namespace SpriteEditor
{
FloorEditStage::FloorEditStage(SpriteDataModel& inSpriteDataModel,
                               const LibraryWindow& inLibraryWindow)
: AUI::Window({320, 58, 1297, 1022}, "FloorEditStage")
, spriteDataModel{inSpriteDataModel}
, libraryWindow{inLibraryWindow}
, activeFloorID{SDL_MAX_UINT16}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, modifyText{{277, 58, 800, 24}, "ModifyText"}
, clearText{{277, 88, 800, 24}, "ClearText"}
, spriteContainer{{559, 181, 180, 255}, "SpriteContainer"}
, descText1{{24, 806, 1240, 24}, "DescText1"}
, descText2{{24, 846, 1240, 24}, "DescText2"}
, descText3{{24, 886, 1240, 24}, "DescText3"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(modifyText);
    children.push_back(clearText);
    children.push_back(spriteContainer);
    children.push_back(descText1);
    children.push_back(descText2);
    children.push_back(descText3);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Floor Sprite Set");

    styleText(modifyText);
    modifyText.setText("To modify: select a sprite in the Library window, then "
                       "press one of the ASSIGN buttons.");
    styleText(clearText);
    clearText.setText("To clear: click an empty area to clear your selection, "
                      "then press one of the ASSIGN buttons.");
    styleText(descText1);
    descText1.setText("Floors are the lowest layer on a tile. They're rendered "
                      "below everything else.");
    styleText(descText2);
    descText2.setText("Floors have no collision, regardless of their sprite's "
                      "collisionEnabled.");
    styleText(descText3);
    descText3.setText(
        "There's no need to draw a bounding box on floor sprites. Their tile "
        "will be used for mouse hit detection.");

    /* Container */
    spriteContainer.setNumColumns(1);
    spriteContainer.setCellWidth(180);
    spriteContainer.setCellHeight(255 + 14);

    // When the active sprite set is updated, update it in this widget.
    spriteDataModel.activeLibraryItemChanged
        .connect<&FloorEditStage::onActiveLibraryItemChanged>(*this);
    spriteDataModel.spriteSetRemoved.connect<&FloorEditStage::onSpriteSetRemoved>(
        *this);
    spriteDataModel.spriteSetSlotChanged
        .connect<&FloorEditStage::onSpriteSetSlotChanged>(*this);
}

void FloorEditStage::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a floor and return early if not.
    const auto* newActiveFloor{
        std::get_if<EditorFloorSpriteSet>(&newActiveItem)};
    if (newActiveFloor == nullptr) {
        activeFloorID = SDL_MAX_UINT16;
        return;
    }

    activeFloorID = newActiveFloor->numericID;

    // Fill the container with the floor's sprites.
    spriteContainer.clear();
    for (std::size_t i = 0; i < newActiveFloor->spriteIDs.size(); ++i) {
        int spriteID{newActiveFloor->spriteIDs[i]};

        // Construct the new slot widget.
        std::unique_ptr<AUI::Widget> slotPtr{
            std::make_unique<SpriteSetSlot>("FloorSpriteSlot")};
        SpriteSetSlot& slot{static_cast<SpriteSetSlot&>(*slotPtr)};

        // Set the top text.
        slot.topText.setText(getTopText(i));

        // Fill in the sprite's data.
        fillSlotSpriteData(slot, spriteID);

        spriteContainer.push_back(std::move(slotPtr));
    }

    // Make sure the container is visible.
    spriteContainer.setIsVisible(true);
}

void FloorEditStage::onSpriteSetRemoved(SpriteSet::Type type, Uint16 spriteSetID)
{
    if ((type == SpriteSet::Type::Floor) && (spriteSetID == activeFloorID)) {
        activeFloorID = SDL_MAX_UINT16;
        spriteContainer.setIsVisible(false);
    }
}

void FloorEditStage::onSpriteSetSlotChanged(SpriteSet::Type type,
                                            Uint16 spriteSetID,
                                            std::size_t index, int newSpriteID)
{
    // If the changed data doesn't affect us, return early.
    if ((type != SpriteSet::Type::Floor) || (spriteSetID != activeFloorID)) {
        return;
    }
    if (index > spriteContainer.size()) {
        LOG_FATAL("Index out of bounds.");
    }

    // Fill in the new slot data.
    SpriteSetSlot& slot{static_cast<SpriteSetSlot&>(*spriteContainer[index])};
    fillSlotSpriteData(slot, newSpriteID);
}

void FloorEditStage::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

std::string FloorEditStage::getTopText(std::size_t spriteSetIndex)
{
    // Floors don't have any top text.
    return "";
}

void FloorEditStage::fillSlotSpriteData(SpriteSetSlot& slot, int spriteID)
{
    // If this slot isn't empty, set the widget's data.
    if (spriteID != EMPTY_SPRITE_ID) {
        // Calc a square texture extent that shows the bottom of the sprite
        // (so we don't have to squash it).
        const EditorSprite& sprite{spriteDataModel.getSprite(spriteID)};
        SDL_Rect textureExtent{sprite.textureExtent};
        if (textureExtent.h > textureExtent.w) {
            int diff{textureExtent.h - textureExtent.w};
            textureExtent.h -= diff;
            textureExtent.y += diff;
        }

        // Load the sprite's image into the slot.
        std::string imagePath{spriteDataModel.getWorkingTexturesDir()};
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

    // Set the callback.
    slot.assignButton.setOnPressed([this, &slot]() {
        const auto& selectedListItems{libraryWindow.getSelectedListItems()};
        for (const LibraryListItem* selectedItem : selectedListItems) {
            // If this is a sprite, fill this slot with its data.
            if (selectedItem->type == LibraryListItem::Type::Sprite) {
                fillSlotSpriteData(slot, selectedItem->ID);
            }
        }
    });
}

} // End namespace SpriteEditor
} // End namespace AM
