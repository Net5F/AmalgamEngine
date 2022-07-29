#include "SpriteEditStage.h"
#include "MainScreen.h"
#include "Sprite.h"
#include "AssetCache.h"
#include "SpriteDataModel.h"
#include "Paths.h"
#include "Ignore.h"
#include "AUI/Core.h"

namespace AM
{
namespace SpriteEditor
{
SpriteEditStage::SpriteEditStage(AssetCache& inAssetCache,
                                 SpriteDataModel& inSpriteDataModel)
: AUI::Window({389, 60, 1142, 684}, "SpriteEditStage")
, assetCache{inAssetCache}
, spriteDataModel{inSpriteDataModel}
, activeSpriteID{SpriteDataModel::INVALID_SPRITE_ID}
, checkerboardImage({0, 0, 100, 100})
, spriteImage({0, 0, 100, 100})
, boundingBoxGizmos{}
{
    // Reserve our gizmo memory so that references don't get invalidated by 
    // vector re-allocation.
    boundingBoxGizmos.reserve(SharedConfig::MAX_SPRITE_BOUNDING_BOXES);

    // Add our children so they're included in rendering, etc.
    children.push_back(checkerboardImage);
    children.push_back(spriteImage);

    /* Active sprite and checkerboard background. */
    checkerboardImage.addResolution(
        {1920, 1080},
        assetCache.loadTexture(Paths::TEXTURE_DIR
                               + "SpriteEditStage/Checkerboard.png"));
    checkerboardImage.setIsVisible(false);
    spriteImage.setIsVisible(false);

    // When the active sprite is updated, update it in this widget.
    spriteDataModel.activeSpriteChanged
        .connect<&SpriteEditStage::onActiveSpriteChanged>(*this);
    spriteDataModel.spriteModelBoundsAdded
        .connect<&SpriteEditStage::onSpriteModelBoundsAdded>(*this);
    spriteDataModel.spriteModelBoundsRemoved
        .connect<&SpriteEditStage::onSpriteModelBoundsRemoved>(*this);
    spriteDataModel.activeSpriteModelBoundsChanged
        .connect<&SpriteEditStage::onActiveSpriteModelBoundsChanged>(*this);
    spriteDataModel.spriteRemoved.connect<&SpriteEditStage::onSpriteRemoved>(
        *this);
}

void SpriteEditStage::onActiveSpriteChanged(unsigned int newActiveSpriteID,
                                            unsigned int newActiveModelBoundsIndex,
                                            const Sprite& newActiveSprite)
{
    activeSpriteID = newActiveSpriteID;

    // Load the sprite's image.
    spriteImage.clearTextures();
    std::string imagePath{spriteDataModel.getWorkingTexturesDir()};
    imagePath += newActiveSprite.parentSpriteSheetPath;
    spriteImage.addResolution(AUI::Core::getLogicalScreenSize(),
                              assetCache.loadTexture(imagePath),
                              newActiveSprite.textureExtent);

    // Calc the centered sprite position.
    SDL_Rect centeredSpriteExtent{newActiveSprite.textureExtent};
    centeredSpriteExtent.x = logicalExtent.w / 2;
    centeredSpriteExtent.x -= (centeredSpriteExtent.w / 2);
    centeredSpriteExtent.y = logicalExtent.h / 2;
    centeredSpriteExtent.y -= (centeredSpriteExtent.h / 2);

    // Size the sprite image to the sprite extent size.
    spriteImage.setLogicalExtent(centeredSpriteExtent);

    // Set the background to the size of the sprite.
    checkerboardImage.setLogicalExtent(spriteImage.getLogicalExtent());

    // Clear out the old bounding boxes.
    const std::size_t elementsToClear{boundingBoxGizmos.size()};
    for (std::size_t i = 0; i < elementsToClear; ++i) {
        boundingBoxGizmos.pop_back();
        children.pop_back();
    }

    // Add a BoundingBoxGizmo child widget for each BoundingBox that the 
    // sprite has.
    for (unsigned int i = 0; i < newActiveSprite.modelBounds.size(); ++i) {
        BoundingBoxGizmo& gizmo{
            boundingBoxGizmos.emplace_back(spriteDataModel, activeSpriteID, i)};
        gizmo.setLogicalExtent(spriteImage.getLogicalExtent());
        gizmo.setIsVisible(true);

        // Select the gizmo associatd with the active bounds.
        if (i == newActiveModelBoundsIndex) {
            gizmo.setIsSelected(true);
        }

        children.push_back(gizmo);
    }

    // Set the sprite and background to be visible.
    checkerboardImage.setIsVisible(true);
    spriteImage.setIsVisible(true);
}

void SpriteEditStage::onSpriteModelBoundsAdded(unsigned int spriteID,
                              unsigned int addedBoundsIndex,
                              const BoundingBox& newModelBounds)
{
    if (spriteID != activeSpriteID) {
        // (We only support removing from the current sprite.)
        LOG_FATAL(
            "Tried to remove from non-current sprite (this shouldn't happen).");
    }
    else if (addedBoundsIndex != boundingBoxGizmos.size()) {
        // (We only support adding boxes to the end of the list.)
        LOG_FATAL("Incorrect gizmo count (this shouldn't happen).");
    }

    // Add a BoundingBoxGizmo child widget for the new bounding box.
    BoundingBoxGizmo& gizmo{boundingBoxGizmos.emplace_back(
        spriteDataModel, spriteID, addedBoundsIndex)};
    gizmo.setLogicalExtent(spriteImage.getLogicalExtent());
    gizmo.setIsVisible(true);

    children.push_back(gizmo);
}

void SpriteEditStage::onSpriteModelBoundsRemoved(unsigned int spriteID,
                                unsigned int removedBoundsIndex)
{
    if (spriteID != activeSpriteID) {
        // (We only support removing from the current sprite.)
        LOG_FATAL(
            "Tried to remove from non-current sprite (this shouldn't happen).");
    }
    else if (removedBoundsIndex != (boundingBoxGizmos.size() - 1)) {
        // (We only support removing boxes from the end of the list.)
        LOG_FATAL("Incorrect gizmo count (this shouldn't happen).");
    }

    // Find and remove the associated gizmo from our children vector.
    // (Our children can be reordered, so we can't assume the removed 
    // bounds will be at the back.)
    for (auto it = children.begin(); it != children.end(); ++it) {
        if (&(it->get()) == &(boundingBoxGizmos.back())) {
            children.erase(it);
            break;
        }
    }

    // Check if the gizmo that we're going to remove is selected.
    boundingBoxGizmos.back().getIsSelected();

    // Remove the associated gizmo.
    boundingBoxGizmos.pop_back();
}

void SpriteEditStage::onActiveSpriteModelBoundsChanged(unsigned int newActiveModelBoundsIndex,
                const BoundingBox& newActiveModelBounds)
{
    ignore(newActiveModelBounds);

    // Activate the gizmo associated with the new active bounds.
    auto activeGizmoIt{selectGizmoAtIndex(newActiveModelBoundsIndex)};

    // Move the active gizmo to the back (so it gets rendered on top).
    std::rotate(activeGizmoIt, activeGizmoIt + 1, children.end());
}

void SpriteEditStage::onSpriteRemoved(unsigned int spriteID)
{
    // If the active sprite was removed.
    if (spriteID == activeSpriteID) {
        activeSpriteID = SpriteDataModel::INVALID_SPRITE_ID;

        // Set everything back to being invisible.
        checkerboardImage.setIsVisible(false);
        spriteImage.setIsVisible(false);

        // Remove all the BoundingBoxGizmo child widgets.
        for (unsigned int i = 0; i < boundingBoxGizmos.size(); ++i) {
            boundingBoxGizmos.pop_back();
            children.pop_back();
        }
    }
}

std::vector<std::reference_wrapper<AUI::Widget>>::iterator
 SpriteEditStage::selectGizmoAtIndex(unsigned int modelBoundsIndex)
{
    if (modelBoundsIndex >= boundingBoxGizmos.size()) {
        LOG_FATAL("Tried to access out of bounds gizmo.");
    }

    // Select the gizmo associated with the given bounds.
    // Note: We skip the first 2 indices of children since they aren't gizmos.
    BoundingBoxGizmo& activeGizmo{boundingBoxGizmos[modelBoundsIndex]};
    auto activeGizmoIt{children.end()};
    for (auto it = children.begin() + 2; it != children.end(); ++it) {
        BoundingBoxGizmo& gizmo{static_cast<BoundingBoxGizmo&>(it->get())};

        // If this is the gizmo that needs to be select, select it.
        if (&gizmo == &activeGizmo) {
            gizmo.setIsSelected(true);
            activeGizmoIt = it;
        }
        else {
            // Deselect all other gizmos.
            gizmo.setIsSelected(false);
        }
    }

    if (activeGizmoIt == children.end()) {
        LOG_FATAL("Failed to find expected gizmo.");
    }

    return activeGizmoIt;
}

} // End namespace SpriteEditor
} // End namespace AM
