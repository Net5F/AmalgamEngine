#include "BoundingBoxGizmo.h"
#include "MainScreen.h"
#include "SpriteDataModel.h"
#include "Transforms.h"
#include "Position.h"
#include "Camera.h"
#include "SharedConfig.h"
#include "Ignore.h"
#include "Log.h"
#include "AUI/Core.h"
#include "AUI/ScalingHelpers.h"
#include "AUI/SDLHelpers.h"
#include <algorithm>

#include <SDL2_gfxPrimitives.h>

namespace AM
{
namespace SpriteEditor
{
BoundingBoxGizmo::BoundingBoxGizmo(SpriteDataModel& inSpriteDataModel)
: AUI::Widget({0, 0, 1920, 1080}, "BoundingBoxGizmo")
, spriteDataModel{inSpriteDataModel}
, activeSpriteID{SpriteDataModel::INVALID_SPRITE_ID}
, scaledRectSize{AUI::ScalingHelpers::logicalToActual(LOGICAL_RECT_SIZE)}
, scaledLineWidth{AUI::ScalingHelpers::logicalToActual(LOGICAL_LINE_WIDTH)}
, hasBoundingBox{false}
, positionControlExtent{0, 0, scaledRectSize, scaledRectSize}
, lastRenderedPosExtent{}
, xControlExtent{0, 0, scaledRectSize, scaledRectSize}
, lastRenderedXExtent{}
, yControlExtent{0, 0, scaledRectSize, scaledRectSize}
, lastRenderedYExtent{}
, zControlExtent{0, 0, scaledRectSize, scaledRectSize}
, lastRenderedZExtent{}
, xMinPoint{}
, xMaxPoint{}
, yMinPoint{}
, yMaxPoint{}
, zMinPoint{}
, zMaxPoint{}
, planeXCoords{}
, planeYCoords{}
, currentHeldControl{Control::None}
{
    // When the active sprite is updated, update it in this widget.
    spriteDataModel.activeSpriteChanged
        .connect<&BoundingBoxGizmo::onActiveSpriteChanged>(*this);
    spriteDataModel.spriteHasBoundingBoxChanged
        .connect<&BoundingBoxGizmo::onSpriteHasBoundingBoxChanged>(*this);
    spriteDataModel.spriteModelBoundsChanged
        .connect<&BoundingBoxGizmo::onSpriteModelBoundsChanged>(*this);
}

void BoundingBoxGizmo::render()
{
    // Render the planes.
    renderPlanes();

    // Render the lines.
    renderLines();

    // Render the control rectangles.
    renderControls();
}

AUI::EventResult BoundingBoxGizmo::onMouseDown(AUI::MouseButtonType buttonType,
                                               const SDL_Point& cursorPosition)
{
    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // Check if the mouse press hit any of our controls.
    if (AUI::SDLHelpers::pointInRect(cursorPosition, lastRenderedPosExtent)) {
        currentHeldControl = Control::Position;
    }
    else if (AUI::SDLHelpers::pointInRect(cursorPosition,
                                          lastRenderedXExtent)) {
        currentHeldControl = Control::X;
    }
    else if (AUI::SDLHelpers::pointInRect(cursorPosition,
                                          lastRenderedYExtent)) {
        currentHeldControl = Control::Y;
    }
    else if (AUI::SDLHelpers::pointInRect(cursorPosition,
                                          lastRenderedZExtent)) {
        currentHeldControl = Control::Z;
    }

    // If we're holding a control, set mouse capture so we get the associated
    // MouseUp.
    if (currentHeldControl != Control::None) {
        return AUI::EventResult{.wasHandled{true}, .setMouseCapture{this}};
    }
    else {
        return AUI::EventResult{.wasHandled{true}};
    }
}

AUI::EventResult BoundingBoxGizmo::onMouseUp(AUI::MouseButtonType buttonType,
                                             const SDL_Point& cursorPosition)
{
    ignore(cursorPosition);

    // Only respond to the left mouse button.
    if (buttonType != AUI::MouseButtonType::Left) {
        return AUI::EventResult{.wasHandled{false}};
    }

    // If we're holding a control, release it and release mouse capture.
    if (currentHeldControl != Control::None) {
        currentHeldControl = Control::None;
        return AUI::EventResult{.wasHandled{true}, .releaseMouseCapture{true}};
    }
    else {
        return AUI::EventResult{.wasHandled{true}};
    }
}

AUI::EventResult BoundingBoxGizmo::onMouseMove(const SDL_Point& cursorPosition)
{
    // If we aren't being pressed, ignore the event.
    if (currentHeldControl == Control::None) {
        return AUI::EventResult{.wasHandled{false}};
    }

    /* Translate the mouse position to world space. */
    // Account for the sprite's empty vertical space.
    const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
    int yOffset{AUI::ScalingHelpers::logicalToActual(activeSprite.yOffset)};
    yOffset += renderExtent.y;

    // Account for the sprite's half-tile offset.
    int xOffset{AUI::ScalingHelpers::logicalToActual(
        static_cast<int>(SharedConfig::TILE_SCREEN_WIDTH / 2.f))};
    xOffset += renderExtent.x;

    // Apply the offset to the mouse position and convert to logical space.
    SDL_Point offsetMousePoint{cursorPosition.x - xOffset,
                               cursorPosition.y - yOffset};
    offsetMousePoint = AUI::ScalingHelpers::actualToLogical(offsetMousePoint);

    // Convert the screen-space mouse point to world space.
    ScreenPoint offsetMouseScreenPoint{static_cast<float>(offsetMousePoint.x),
                                       static_cast<float>(offsetMousePoint.y)};
    Position mouseWorldPos{
        Transforms::screenToWorld(offsetMouseScreenPoint, {})};

    // Adjust the currently pressed control appropriately.
    switch (currentHeldControl) {
        case Control::Position: {
            updatePositionBounds(mouseWorldPos);
            break;
        }
        case Control::X: {
            updateXBounds(mouseWorldPos);
            break;
        }
        case Control::Y: {
            updateYBounds(mouseWorldPos);
            break;
        }
        case Control::Z: {
            updateZBounds(cursorPosition.y);
            break;
        }
        default: {
            break;
        }
    }

    return AUI::EventResult{.wasHandled{true}};
}

bool BoundingBoxGizmo::refreshScaling()
{
    // If scaledExtent was refreshed, do our specialized refreshing.
    if (Widget::refreshScaling()) {
        // Re-calculate our control rectangle size.
        scaledRectSize
            = AUI::ScalingHelpers::logicalToActual(LOGICAL_RECT_SIZE);

        // Re-calculate our line width.
        scaledLineWidth
            = AUI::ScalingHelpers::logicalToActual(LOGICAL_LINE_WIDTH);

        // Refresh our controls to reflect the new sizes.
        refresh(spriteDataModel.getSprite(activeSpriteID));

        return true;
    }

    return false;
}

void BoundingBoxGizmo::onActiveSpriteChanged(unsigned int newActiveSpriteID,
                                             const Sprite& newActiveSprite)
{
    activeSpriteID = newActiveSpriteID;
    hasBoundingBox = newActiveSprite.hasBoundingBox;
    refresh(newActiveSprite);
}

void BoundingBoxGizmo::onSpriteHasBoundingBoxChanged(unsigned int spriteID,
                                                     bool newHasBoundingBox)
{
    if (spriteID == activeSpriteID) {
        hasBoundingBox = newHasBoundingBox;
    }
}

void BoundingBoxGizmo::onSpriteModelBoundsChanged(
    unsigned int spriteID, const BoundingBox& newModelBounds)
{
    ignore(newModelBounds);

    if (spriteID == activeSpriteID) {
        refresh(spriteDataModel.getSprite(spriteID));
    }
}

void BoundingBoxGizmo::refresh(const Sprite& activeSprite)
{
    // Calculate where the sprite's model bounds are on the screen.
    // Note: The ordering of the points in this vector is listed in the comment
    //       for calcOffsetScreenPoints().
    std::vector<SDL_Point> boundsScreenPoints;
    calcOffsetScreenPoints(activeSprite, boundsScreenPoints);

    // Move the controls to the correct positions.
    moveControls(boundsScreenPoints);

    // Move the lines to the correct positions.
    moveLines(boundsScreenPoints);

    // Move the planes to the correct positions.
    movePlanes(boundsScreenPoints);
}

void BoundingBoxGizmo::updatePositionBounds(const Position& mouseWorldPos)
{
    // Note: The expected behavior is to move along the x/y plane and
    //       leave minZ where it was.
    const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
    BoundingBox modelBounds{activeSprite.modelBounds};
    float& minX{modelBounds.minX};
    float& minY{modelBounds.minY};
    float& maxX{modelBounds.maxX};
    float& maxY{modelBounds.maxY};

    // Move the min bounds to follow the max bounds.
    float diffX{mouseWorldPos.x - maxX};
    float diffY{mouseWorldPos.y - maxY};
    minX += diffX;
    minY += diffY;

    // Move the max bounds to their new position.
    maxX = mouseWorldPos.x;
    maxY = mouseWorldPos.y;

    // If we moved below the model-space origin (0, 0), bring the box bounds
    // back in.
    if (minX < 0) {
        maxX += -minX;
        minX = 0;
    }
    if (minY < 0) {
        maxY += -minY;
        minY = 0;
    }

    // If we moved outside the tile bounds, bring the box bounds back in.
    if (maxX > SharedConfig::TILE_WORLD_WIDTH) {
        float diff{maxX - SharedConfig::TILE_WORLD_WIDTH};
        minX -= diff;
        maxX -= diff;
    }
    if (maxY > SharedConfig::TILE_WORLD_WIDTH) {
        float diff{maxY - SharedConfig::TILE_WORLD_WIDTH};
        minY -= diff;
        maxY -= diff;
    }

    // Apply the new model bounds.
    spriteDataModel.setSpriteModelBounds(activeSpriteID, modelBounds);
}

void BoundingBoxGizmo::updateXBounds(const Position& mouseWorldPos)
{
    // Clamp the new value to its bounds.
    const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
    BoundingBox modelBounds{activeSprite.modelBounds};
    modelBounds.minX = std::clamp(mouseWorldPos.x, 0.f, modelBounds.maxX);

    // Apply the new model bound.
    spriteDataModel.setSpriteModelBounds(activeSpriteID, modelBounds);
}

void BoundingBoxGizmo::updateYBounds(const Position& mouseWorldPos)
{
    // Clamp the new value to its bounds.
    const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
    BoundingBox modelBounds{activeSprite.modelBounds};
    modelBounds.minY = std::clamp(mouseWorldPos.y, 0.f, modelBounds.maxY);

    // Apply the new model bound.
    spriteDataModel.setSpriteModelBounds(activeSpriteID, modelBounds);
}

void BoundingBoxGizmo::updateZBounds(int mouseScreenYPos)
{
    // Note: The screenToWorld() transformation can't handle z-axis
    // movement (not enough data from a 2d point), so we have to do it
    // using our contextual information.

    // Set maxZ relative to the distance between the mouse and the
    // position control (the position control is always our reference
    // for where z == 0 is.)
    float mouseZHeight{lastRenderedPosExtent.y + (scaledRectSize / 2.f)
                       - mouseScreenYPos};

    // Convert to logical space.
    mouseZHeight = AUI::ScalingHelpers::actualToLogical(mouseZHeight);

    // Apply our screen -> world Z scaling.
    mouseZHeight = Transforms::screenYToWorldZ(mouseZHeight, 1.f);

    // Set maxZ, making sure it doesn't go below minZ.
    const Sprite& activeSprite{spriteDataModel.getSprite(activeSpriteID)};
    BoundingBox modelBounds{activeSprite.modelBounds};
    if (mouseZHeight > modelBounds.minZ) {
        modelBounds.maxZ = mouseZHeight;

        // Apply the new model bound.
        spriteDataModel.setSpriteModelBounds(activeSpriteID, modelBounds);
    }
}

void BoundingBoxGizmo::calcOffsetScreenPoints(
    const Sprite& activeSprite, std::vector<SDL_Point>& boundsScreenPoints)
{
    /* Transform the world positions to screen points. */
    // Set up a vector of float points so we can maintain precision until
    // the end.
    std::array<ScreenPoint, 7> floatPoints{};

    // Push the points in the correct order.
    const BoundingBox& modelBounds{activeSprite.modelBounds};
    Position position{modelBounds.minX, modelBounds.maxY, modelBounds.minZ};
    floatPoints[0] = Transforms::worldToScreen(position, 1);

    position = {modelBounds.maxX, modelBounds.maxY, modelBounds.minZ};
    floatPoints[1] = Transforms::worldToScreen(position, 1);

    position = {modelBounds.maxX, modelBounds.minY, modelBounds.minZ};
    floatPoints[2] = Transforms::worldToScreen(position, 1);

    position = {modelBounds.minX, modelBounds.maxY, modelBounds.maxZ};
    floatPoints[3] = Transforms::worldToScreen(position, 1);

    position = {modelBounds.maxX, modelBounds.maxY, modelBounds.maxZ};
    floatPoints[4] = Transforms::worldToScreen(position, 1);

    position = {modelBounds.maxX, modelBounds.minY, modelBounds.maxZ};
    floatPoints[5] = Transforms::worldToScreen(position, 1);

    position = {modelBounds.minX, modelBounds.minY, modelBounds.maxZ};
    floatPoints[6] = Transforms::worldToScreen(position, 1);

    /* Build the offsets. */
    // Account for the sprite's empty vertical space.
    int yOffset{AUI::ScalingHelpers::logicalToActual(activeSprite.yOffset)};

    // Account for the sprite's half-tile offset.
    int xOffset{AUI::ScalingHelpers::logicalToActual(
        static_cast<int>(activeSprite.textureExtent.w / 2.f))};

    /* Scale and offset each point, then push it into the return vector. */
    for (ScreenPoint& point : floatPoints) {
        // Scale and round the point.
        point.x = std::round(AUI::ScalingHelpers::logicalToActual(point.x));
        point.y = std::round(AUI::ScalingHelpers::logicalToActual(point.y));

        // Offset the point.
        point.x += xOffset;
        point.y += yOffset;

        // Cast to int and push into the return vector.
        boundsScreenPoints.push_back(
            {static_cast<int>(point.x), static_cast<int>(point.y)});
    }
}

void BoundingBoxGizmo::moveControls(std::vector<SDL_Point>& boundsScreenPoints)
{
    // Calc half the control rectangle size so we can center the controls.
    int halfRectSize{static_cast<int>(scaledRectSize / 2.f)};

    // Move the control extents.
    positionControlExtent.x = boundsScreenPoints[1].x - halfRectSize;
    positionControlExtent.y = boundsScreenPoints[1].y - halfRectSize;

    xControlExtent.x = boundsScreenPoints[0].x - halfRectSize;
    xControlExtent.y = boundsScreenPoints[0].y - halfRectSize;

    yControlExtent.x = boundsScreenPoints[2].x - halfRectSize;
    yControlExtent.y = boundsScreenPoints[2].y - halfRectSize;

    zControlExtent.x = boundsScreenPoints[4].x - halfRectSize;
    zControlExtent.y = boundsScreenPoints[4].y - halfRectSize;
}

void BoundingBoxGizmo::moveLines(std::vector<SDL_Point>& boundsScreenPoints)
{
    // Move the lines.
    xMinPoint = {boundsScreenPoints[0].x, boundsScreenPoints[0].y};
    xMaxPoint = {boundsScreenPoints[1].x, boundsScreenPoints[1].y};

    yMinPoint = {boundsScreenPoints[2].x, boundsScreenPoints[2].y};
    yMaxPoint = {boundsScreenPoints[1].x, boundsScreenPoints[1].y};

    zMinPoint = {boundsScreenPoints[1].x, boundsScreenPoints[1].y};
    zMaxPoint = {boundsScreenPoints[4].x, boundsScreenPoints[4].y};
}

void BoundingBoxGizmo::movePlanes(std::vector<SDL_Point>& boundsScreenPoints)
{
    // Set the coords for the X-axis plane (coords 0 - 3, starting from top
    // left and going clockwise.)
    planeXCoords[0] = boundsScreenPoints[4].x;
    planeYCoords[0] = boundsScreenPoints[4].y;
    planeXCoords[1] = boundsScreenPoints[5].x;
    planeYCoords[1] = boundsScreenPoints[5].y;
    planeXCoords[2] = boundsScreenPoints[2].x;
    planeYCoords[2] = boundsScreenPoints[2].y;
    planeXCoords[3] = boundsScreenPoints[1].x;
    planeYCoords[3] = boundsScreenPoints[1].y;

    // Set the coords for the Y-axis plane (coords 4 - 7, starting from top
    // left and going clockwise.)
    planeXCoords[4] = boundsScreenPoints[3].x;
    planeYCoords[4] = boundsScreenPoints[3].y;
    planeXCoords[5] = boundsScreenPoints[4].x;
    planeYCoords[5] = boundsScreenPoints[4].y;
    planeXCoords[6] = boundsScreenPoints[1].x;
    planeYCoords[6] = boundsScreenPoints[1].y;
    planeXCoords[7] = boundsScreenPoints[0].x;
    planeYCoords[7] = boundsScreenPoints[0].y;

    // Set the coords for the Z-axis plane (coords 8 - 11, starting from top
    // left and going clockwise.)
    planeXCoords[8] = boundsScreenPoints[6].x;
    planeYCoords[8] = boundsScreenPoints[6].y;
    planeXCoords[9] = boundsScreenPoints[5].x;
    planeYCoords[9] = boundsScreenPoints[5].y;
    planeXCoords[10] = boundsScreenPoints[4].x;
    planeYCoords[10] = boundsScreenPoints[4].y;
    planeXCoords[11] = boundsScreenPoints[3].x;
    planeYCoords[11] = boundsScreenPoints[3].y;
}

void BoundingBoxGizmo::renderControls()
{
    // If the bounding box is disabled, show it at 1/4 alpha.
    float alpha{255};
    if (!hasBoundingBox) {
        alpha /= 4.f;
    }

    // Position control
    SDL_Rect offsetExtent{positionControlExtent};
    offsetExtent.x += renderExtent.x;
    offsetExtent.y += renderExtent.y;
    lastRenderedPosExtent = offsetExtent;

    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 0, 0, 0,
                           static_cast<Uint8>(alpha));
    SDL_RenderFillRect(AUI::Core::getRenderer(), &lastRenderedPosExtent);

    // X control
    offsetExtent = xControlExtent;
    offsetExtent.x += renderExtent.x;
    offsetExtent.y += renderExtent.y;
    lastRenderedXExtent = offsetExtent;

    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 148, 0, 0,
                           static_cast<Uint8>(alpha));
    SDL_RenderFillRect(AUI::Core::getRenderer(), &lastRenderedXExtent);

    // Y control
    offsetExtent = yControlExtent;
    offsetExtent.x += renderExtent.x;
    offsetExtent.y += renderExtent.y;
    lastRenderedYExtent = offsetExtent;

    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 0, 149, 0,
                           static_cast<Uint8>(alpha));
    SDL_RenderFillRect(AUI::Core::getRenderer(), &lastRenderedYExtent);

    // Z control
    offsetExtent = zControlExtent;
    offsetExtent.x += renderExtent.x;
    offsetExtent.y += renderExtent.y;
    lastRenderedZExtent = offsetExtent;

    SDL_SetRenderDrawColor(AUI::Core::getRenderer(), 0, 82, 240,
                           static_cast<Uint8>(alpha));
    SDL_RenderFillRect(AUI::Core::getRenderer(), &lastRenderedZExtent);
}

void BoundingBoxGizmo::renderLines()
{
    // If the bounding box is disabled, show it at 1/4 alpha.
    float alpha{255};
    if (!hasBoundingBox) {
        alpha /= 4.f;
    }

    // X-axis line
    SDL_Point offsetMinPoint{xMinPoint};
    SDL_Point offsetMaxPoint{xMaxPoint};
    offsetMinPoint.x += renderExtent.x;
    offsetMinPoint.y += renderExtent.y;
    offsetMaxPoint.x += renderExtent.x;
    offsetMaxPoint.y += renderExtent.y;

    thickLineRGBA(AUI::Core::getRenderer(), offsetMinPoint.x, offsetMinPoint.y,
                  offsetMaxPoint.x, offsetMaxPoint.y, scaledLineWidth, 148, 0,
                  0, static_cast<Uint8>(alpha));

    // Y-axis line
    offsetMinPoint = yMinPoint;
    offsetMaxPoint = yMaxPoint;
    offsetMinPoint.x += renderExtent.x;
    offsetMinPoint.y += renderExtent.y;
    offsetMaxPoint.x += renderExtent.x;
    offsetMaxPoint.y += renderExtent.y;

    thickLineRGBA(AUI::Core::getRenderer(), offsetMinPoint.x, offsetMinPoint.y,
                  offsetMaxPoint.x, offsetMaxPoint.y, scaledLineWidth, 0, 149,
                  0, static_cast<Uint8>(alpha));

    // Z-axis line
    offsetMinPoint = zMinPoint;
    offsetMaxPoint = zMaxPoint;
    offsetMinPoint.x += renderExtent.x;
    offsetMinPoint.y += renderExtent.y;
    offsetMaxPoint.x += renderExtent.x;
    offsetMaxPoint.y += renderExtent.y;

    thickLineRGBA(AUI::Core::getRenderer(), offsetMinPoint.x, offsetMinPoint.y,
                  offsetMaxPoint.x, offsetMaxPoint.y, scaledLineWidth, 0, 82,
                  240, static_cast<Uint8>(alpha));
}

void BoundingBoxGizmo::renderPlanes()
{
    /* Offset all the points. */
    std::array<Sint16, 12> offsetXCoords{};
    for (unsigned int i = 0; i < offsetXCoords.size(); ++i) {
        offsetXCoords[i] = planeXCoords[i] + renderExtent.x;
    }

    std::array<Sint16, 12> offsetYCoords{};
    for (unsigned int i = 0; i < offsetYCoords.size(); ++i) {
        offsetYCoords[i] = planeYCoords[i] + renderExtent.y;
    }

    /* Draw the planes. */
    // If the bounding box is disabled, show it at 1/4 alpha.
    float alpha{127};
    if (!hasBoundingBox) {
        alpha /= 4.f;
    }

    // X-axis plane
    filledPolygonRGBA(AUI::Core::getRenderer(), &(offsetXCoords[0]),
                      &(offsetYCoords[0]), 4, 148, 0, 0,
                      static_cast<Uint8>(alpha));

    // Y-axis plane
    filledPolygonRGBA(AUI::Core::getRenderer(), &(offsetXCoords[4]),
                      &(offsetYCoords[4]), 4, 0, 149, 0,
                      static_cast<Uint8>(alpha));

    // Z-axis plane
    filledPolygonRGBA(AUI::Core::getRenderer(), &(offsetXCoords[8]),
                      &(offsetYCoords[8]), 4, 0, 82, 240,
                      static_cast<Uint8>(alpha));
}

} // End namespace SpriteEditor
} // End namespace AM
