#include "SpriteEditView.h"
#include "MainScreen.h"
#include "EditorSprite.h"
#include "DataModel.h"
#include "SpriteID.h"
#include "Paths.h"
#include "Transforms.h"
#include "SpriteTools.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"
#include <SDL2_gfxPrimitives.h>

namespace AM
{
namespace ResourceImporter
{
SpriteEditView::SpriteEditView(DataModel& inDataModel)
: AUI::Window({320, 58, 1297, 1022}, "SpriteEditView")
, dataModel{inDataModel}
, activeSpriteID{NULL_SPRITE_ID}
, stageXCoords{}
, stageYCoords{}
, topText{{0, 0, logicalExtent.w, 34}, "TopText"}
, checkerboardImage{{0, 0, 100, 100}, "BackgroundImage"}
, spriteImage{{0, 0, 100, 100}, "SpriteImage"}
, boundingBoxGizmo{{0, 52, 1297, 732}, inDataModel}
, descText{{24, 806, 1240, 200}, "DescText"}
{
    // Add our children so they're included in rendering, etc.
    children.push_back(topText);
    children.push_back(checkerboardImage);
    children.push_back(spriteImage);
    children.push_back(boundingBoxGizmo);
    children.push_back(descText);

    /* Text */
    topText.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 26);
    topText.setColor({255, 255, 255, 255});
    topText.setHorizontalAlignment(AUI::Text::HorizontalAlignment::Center);
    topText.setText("Sprite");

    styleText(descText);
    descText.setText(
        "Sprites are the basic building block for graphics in The Amalgam "
        "Engine.\n\nThe bounding box that you set for this sprite will be used "
        "for render sorting, mouse hit detection, and (if enabled) "
        "collision.\n\nSprites must be added to a Sprite Set to be used in the "
        "engine.\n\nSprite Sets come in various types: Terrain, Floor, Wall, "
        "Object, and Entity.");

    /* Active sprite and checkerboard background. */
    checkerboardImage.setTiledImage(Paths::TEXTURE_DIR
                                    + "SpriteEditView/Checkerboard.png");
    checkerboardImage.setIsVisible(false);
    spriteImage.setIsVisible(false);

    /* Bounding box gizmo. */
    boundingBoxGizmo.setIsVisible(false);

    // When the active sprite is updated, update it in this widget.
    dataModel.activeLibraryItemChanged
        .connect<&SpriteEditView::onActiveLibraryItemChanged>(*this);
    dataModel.spriteModel.spriteModelBoundsIDChanged
        .connect<&SpriteEditView::onSpriteModelBoundsIDChanged>(*this);
    dataModel.spriteModel.spriteCustomModelBoundsChanged
        .connect<&SpriteEditView::onSpriteCustomModelBoundsChanged>(*this);
    dataModel.spriteModel.spriteRemoved
        .connect<&SpriteEditView::onSpriteRemoved>(*this);

    // When the gizmo updates the active sprite's bounds, push it to the model.
    boundingBoxGizmo.setOnBoundingBoxUpdated(
        [&](const BoundingBox& updatedBounds) {
            onGizmoBoundingBoxUpdated(updatedBounds);
        });
}

void SpriteEditView::render()
{
    // If this widget is fully clipped, don't render it.
    if (SDL_RectEmpty(&clippedExtent)) {
        return;
    }

    // Render each widget manually so that we can render the stage graphic in
    // between the background and the sprite.
    if (topText.getIsVisible()) {
        topText.render({scaledExtent.x, scaledExtent.y});
    }
    if (checkerboardImage.getIsVisible()) {
        checkerboardImage.render({scaledExtent.x, scaledExtent.y});
    }
    if (descText.getIsVisible()) {
        descText.render({scaledExtent.x, scaledExtent.y});
    }
    if (boundingBoxGizmo.getIsVisible()) {
        renderStage({scaledExtent.x, scaledExtent.y});
    }
    if (spriteImage.getIsVisible()) {
        spriteImage.render({scaledExtent.x, scaledExtent.y});
    }
    if (boundingBoxGizmo.getIsVisible()) {
        boundingBoxGizmo.render({scaledExtent.x, scaledExtent.y});
    }
}

void SpriteEditView::onActiveLibraryItemChanged(
    const LibraryItemData& newActiveItem)
{
    // Check if the new active item is a sprite and return early if not.
    const EditorSprite* newActiveSprite{get_if<EditorSprite>(&newActiveItem)};
    if (!newActiveSprite) {
        activeSpriteID = NULL_SPRITE_ID;
        return;
    }

    activeSpriteID = newActiveSprite->numericID;

    // Load the sprite's image.
    std::string imagePath{dataModel.getWorkingTexturesDir()};
    imagePath += newActiveSprite->parentSpriteSheetPath;
    spriteImage.setSimpleImage(imagePath, newActiveSprite->textureExtent);

    // Set up the gizmo with the new sprite's size and data.
    // Note: The sprite's native size is used as the logical size.
    boundingBoxGizmo.setSpriteImageSize(newActiveSprite->textureExtent.w,
                                        newActiveSprite->textureExtent.h);
    boundingBoxGizmo.setStageOrigin(newActiveSprite->stageOrigin);
    boundingBoxGizmo.setBoundingBox(
        newActiveSprite->getModelBounds(dataModel.boundingBoxModel));

    // If the sprite is using a shared bounding box, disable the gizmo.
    if (newActiveSprite->modelBoundsID) {
        boundingBoxGizmo.disable();
    }
    else {
        // The sprite is using custom bounds, enable the gizmo.
        boundingBoxGizmo.enable();
    }

    // Use the gizmo's centered sprite extent to set the background and sprite
    // extents.
    SDL_Rect logicalSpriteExtent{
        boundingBoxGizmo.getLogicalCenteredSpriteExtent()};
    logicalSpriteExtent.x += boundingBoxGizmo.getLogicalExtent().x;
    logicalSpriteExtent.y += boundingBoxGizmo.getLogicalExtent().y;
    checkerboardImage.setLogicalExtent(logicalSpriteExtent);
    spriteImage.setLogicalExtent(logicalSpriteExtent);

    // Make sure everything is visible.
    boundingBoxGizmo.setIsVisible(true);
    checkerboardImage.setIsVisible(true);
    spriteImage.setIsVisible(true);

    // Calculate where the stage is on the screen, for our generated graphics.
    std::vector<SDL_Point> screenPoints{};
    calcStageScreenPoints(screenPoints);

    // Move the stage graphic coords to the correct positions.
    moveStageGraphic(screenPoints);
}

void SpriteEditView::onSpriteModelBoundsIDChanged(
    SpriteID spriteID, BoundingBoxID newModelBoundsID)
{
    // If the sprite isn't active, do nothing.
    if (spriteID != activeSpriteID) {
        return;
    }

    // If the sprite is using a shared bounding box, disable the gizmo.
    if (newModelBoundsID) {
        boundingBoxGizmo.disable();
    }
    else {
        // The sprite is using custom bounds, enable the gizmo.
        boundingBoxGizmo.enable();
    }

    // Whether it's enabled or not, the gizmo should show the correct bounds.
    const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};
    const BoundingBox& newModelBounds{
        sprite.getModelBounds(dataModel.boundingBoxModel)};

    boundingBoxGizmo.setBoundingBox(newModelBounds);
}

void SpriteEditView::onSpriteCustomModelBoundsChanged(
    SpriteID spriteID, const BoundingBox& newCustomModelBounds)
{
    // If the sprite isn't active or isn't set to custom bounds, do nothing.
    const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};
    if ((spriteID != activeSpriteID) || sprite.modelBoundsID) {
        return;
    }

    // Update the gizmo.
    boundingBoxGizmo.setBoundingBox(newCustomModelBounds);
}

void SpriteEditView::onSpriteRemoved(SpriteID spriteID)
{
    if (spriteID == activeSpriteID) {
        activeSpriteID = NULL_SPRITE_ID;

        // Set everything back to being invisible.
        checkerboardImage.setIsVisible(false);
        spriteImage.setIsVisible(false);
        boundingBoxGizmo.setIsVisible(false);
    }
}

void SpriteEditView::onGizmoBoundingBoxUpdated(
    const BoundingBox& updatedBounds)
{
    if (activeSpriteID != NULL_SPRITE_ID) {
        // If the sprite isn't set to use a custom model, do nothing (should
        // never happen since the gizmo should be disabled).
        const EditorSprite& sprite{
            dataModel.spriteModel.getSprite(activeSpriteID)};
        if (sprite.modelBoundsID) {
            return;
        }

        // Update the model with the gizmo's new state.
        dataModel.spriteModel.setSpriteCustomModelBounds(activeSpriteID,
                                                         updatedBounds);
    }
}

void SpriteEditView::styleText(AUI::Text& text)
{
    text.setFont((Paths::FONT_DIR + "B612-Regular.ttf"), 18);
    text.setColor({255, 255, 255, 255});
}

void SpriteEditView::calcStageScreenPoints(
    std::vector<SDL_Point>& stageScreenPoints)
{
    /* Transform the world positions to screen points. */
    std::array<SDL_FPoint, 4> screenPoints{};

    // Push the points in the correct order.
    const EditorSprite& activeSprite{
        dataModel.spriteModel.getSprite(activeSpriteID)};
    BoundingBox stageWorldExtent{SpriteTools::calcSpriteStageWorldExtent(
        activeSprite.textureExtent, activeSprite.stageOrigin)};
    const Vector3& minPoint{stageWorldExtent.min};
    const Vector3& maxPoint{stageWorldExtent.max};
    Vector3 point{minPoint.x, minPoint.y, minPoint.z};
    screenPoints[0] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, minPoint.y, minPoint.z};
    screenPoints[1] = Transforms::worldToScreen(point, 1);

    point = {maxPoint.x, maxPoint.y, minPoint.z};
    screenPoints[2] = Transforms::worldToScreen(point, 1);

    point = {minPoint.x, maxPoint.y, minPoint.z};
    screenPoints[3] = Transforms::worldToScreen(point, 1);

    // Account for the gizmo's position and the image's position.
    const SDL_Rect& gizmoClippedExtent{boundingBoxGizmo.getClippedExtent()};
    SDL_Rect spriteExtent{AUI::ScalingHelpers::logicalToActual(
        boundingBoxGizmo.getLogicalCenteredSpriteExtent())};
    SDL_Point logicalStageOrigin{
        AUI::ScalingHelpers::logicalToActual(activeSprite.stageOrigin)};
    int finalXOffset{gizmoClippedExtent.x + spriteExtent.x
                     + logicalStageOrigin.x};
    int finalYOffset{gizmoClippedExtent.y + spriteExtent.y
                     + logicalStageOrigin.y};

    // Scale and offset each point, then push it into the return vector.
    for (SDL_FPoint& point : screenPoints) {
        // Scale and round the point.
        point.x = std::round(AUI::ScalingHelpers::logicalToActual(point.x));
        point.y = std::round(AUI::ScalingHelpers::logicalToActual(point.y));

        // Offset the point.
        point.x += finalXOffset;
        point.y += finalYOffset;

        // Cast to int and push into the return vector.
        stageScreenPoints.push_back(
            {static_cast<int>(point.x), static_cast<int>(point.y)});
    }
}

void SpriteEditView::moveStageGraphic(
    std::vector<SDL_Point>& stageScreenPoints)
{
    // Set the coords for the bottom face of the stage. (coords 0 - 3, starting
    // from top left and going clockwise.)
    stageXCoords[0] = stageScreenPoints[0].x;
    stageYCoords[0] = stageScreenPoints[0].y;
    stageXCoords[1] = stageScreenPoints[1].x;
    stageYCoords[1] = stageScreenPoints[1].y;
    stageXCoords[2] = stageScreenPoints[2].x;
    stageYCoords[2] = stageScreenPoints[2].y;
    stageXCoords[3] = stageScreenPoints[3].x;
    stageYCoords[3] = stageScreenPoints[3].y;
}

void SpriteEditView::renderStage(const SDL_Point& windowTopLeft)
{
    /* Offset all the points. */
    std::array<Sint16, 4> offsetXCoords{};
    for (std::size_t i = 0; i < offsetXCoords.size(); ++i) {
        offsetXCoords[i] = stageXCoords[i] + windowTopLeft.x;
    }

    std::array<Sint16, 4> offsetYCoords{};
    for (std::size_t i = 0; i < offsetYCoords.size(); ++i) {
        offsetYCoords[i] = stageYCoords[i] + windowTopLeft.y;
    }

    /* Draw the stage's floor bounds. */
    filledPolygonRGBA(AUI::Core::getRenderer(), &(offsetXCoords[0]),
                      &(offsetYCoords[0]), 4, 0, 149, 0,
                      static_cast<Uint8>(STAGE_ALPHA));
}

} // End namespace ResourceImporter
} // End namespace AM
