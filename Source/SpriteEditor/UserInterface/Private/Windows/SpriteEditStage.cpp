#include "SpriteEditStage.h"
#include "MainScreen.h"
#include "Sprite.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "Ignore.h"
#include "AUI/Core.h"

namespace AM
{
namespace SpriteEditor
{
SpriteEditStage::SpriteEditStage(SpriteDataModel& inSpriteDataModel)
: AUI::Window({389, 60, 1142, 684}, "SpriteEditStage")
, spriteDataModel{inSpriteDataModel}
, activeSpriteID{SpriteDataModel::INVALID_SPRITE_ID}
, checkerboardImage({0, 0, 100, 100})
, spriteImage({0, 0, 100, 100})
, boundingBoxGizmo(inSpriteDataModel)
{
    // Add our children so they're included in rendering, etc.
    children.push_back(checkerboardImage);
    children.push_back(spriteImage);
    children.push_back(boundingBoxGizmo);

    /* Active sprite and checkerboard background. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditStage/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    spriteImage.setIsVisible(false);

    /* Bounding box gizmo. */
    boundingBoxGizmo.setIsVisible(false);

    // When the active sprite is updated, update it in this widget.
    spriteDataModel.activeSpriteChanged
        .connect<&SpriteEditStage::onActiveSpriteChanged>(*this);
    // TODO: Switch this to sheet removed
    //spriteDataModel.spriteRemoved.connect<&SpriteEditStage::onSpriteRemoved>(
    //    *this);
}

void SpriteEditStage::onActiveSpriteChanged(unsigned int newActiveSpriteID,
                                            const Sprite& newActiveSprite)
{
    activeSpriteID = newActiveSpriteID;

    // Load the sprite's image.
    std::string imagePath{spriteDataModel.getWorkingTexturesDir()};
    imagePath += newActiveSprite.parentSpriteSheetPath;
    spriteImage.setSimpleImage(imagePath, newActiveSprite.textureExtent);

    // Calc the centered sprite position.
    SDL_Rect centeredSpriteExtent{newActiveSprite.textureExtent};
    centeredSpriteExtent.x = logicalExtent.w / 2;
    centeredSpriteExtent.x -= (centeredSpriteExtent.w / 2);
    centeredSpriteExtent.y = logicalExtent.h / 2;
    centeredSpriteExtent.y -= (centeredSpriteExtent.h / 2);

    // Size the sprite image to the sprite extent size.
    spriteImage.setLogicalExtent(centeredSpriteExtent);

    // Set the background and gizmo to the size of the sprite.
    checkerboardImage.setLogicalExtent(spriteImage.getLogicalExtent());
    boundingBoxGizmo.setLogicalExtent(spriteImage.getLogicalExtent());

    // Set the sprite and background to be visible.
    checkerboardImage.setIsVisible(true);
    spriteImage.setIsVisible(true);

    // If the gizmo isn't visible, make it visible.
    boundingBoxGizmo.setIsVisible(true);
}

void SpriteEditStage::onSpriteRemoved(unsigned int spriteID)
{
    if (spriteID == activeSpriteID) {
        activeSpriteID = SpriteDataModel::INVALID_SPRITE_ID;

        // Set everything back to being invisible.
        checkerboardImage.setIsVisible(false);
        spriteImage.setIsVisible(false);
        boundingBoxGizmo.setIsVisible(false);
    }
}

} // End namespace SpriteEditor
} // End namespace AM
