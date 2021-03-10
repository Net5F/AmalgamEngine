#include "Renderer.h"
#include "Simulation.h"
#include "UserInterface.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Sprite.h"
#include "Camera.h"
#include "BoundingBox.h"
#include "MovementHelpers.h"
#include "TransformationHelpers.h"
#include "ScreenRect.h"
#include "Log.h"
#include "Ignore.h"

#include <SDL2/SDL2_gfxPrimitives.h>

namespace AM
{
namespace Client
{
Renderer::Renderer(SDL2pp::Renderer& inSdlRenderer, SDL2pp::Window& window,
                   Simulation& inSim, UserInterface& inUI, std::function<double(void)> inGetProgress)
: sdlRenderer(inSdlRenderer)
, sim(inSim)
, world(sim.getWorld())
, ui(inUI)
, getProgress(inGetProgress)
{
    // Init the groups that we'll be using.
    auto group
        = world.registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    ignore(group);

    // TODO: This will eventually be used when we get to variable window sizes.
    ignore(window);
}

void Renderer::render()
{
    /* Set up the camera for this frame. */
    // Get how far we are between sim ticks in decimal percent.
    double alpha = getProgress();

    // Get the lerped camera position based on the alpha.
    auto [playerCamera, playerSprite, playerPosition, playerPreviousPos]
        = world.registry.get<Camera, Sprite, Position, PreviousPosition>(
            world.playerEntity);
    Position cameraLerp = MovementHelpers::interpolatePosition(
        playerCamera.prevPosition, playerCamera.position, alpha);

    // Set the camera at the lerped position.
    Camera lerpedCamera = playerCamera;
    lerpedCamera.position.x = cameraLerp.x;
    lerpedCamera.position.y = cameraLerp.y;

    // Get iso screen coords for the center point of camera.
    ScreenPoint lerpedCameraCenter
        = TransformationHelpers::worldToScreen(lerpedCamera.position,
                        lerpedCamera.zoomFactor);

    // Calc where the top left of the lerpedCamera is in screen space.
    lerpedCamera.extent.x
        = lerpedCameraCenter.x - (lerpedCamera.extent.width / 2);
    lerpedCamera.extent.y
        = lerpedCameraCenter.y - (lerpedCamera.extent.height / 2);

    // Save the last calculated screen position of the camera for use in
    // screen -> world calcs.
    playerCamera.extent.x = lerpedCamera.extent.x;
    playerCamera.extent.y = lerpedCamera.extent.y;

    /* Update the world bounds of sprites that are relevant to this frame. */
    updateSpriteWorldBounds(alpha);

    /* Render. */
    // Clear the screen to prepare for drawing.
    sdlRenderer.Clear();

    // Draw all tiles and entities.
    renderWorld(lerpedCamera, alpha);

    // Draw all UI elements.
    renderUserInterface(lerpedCamera);

    // Render the finished buffer to the screen.
    sdlRenderer.Present();
}

bool Renderer::handleEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_WINDOWEVENT:
            // TODO: Handle this.
            return true;
    }

    return false;
}

void Renderer::updateSpriteWorldBounds(const double alpha)
{
    // Update all sprites that are on dynamic (moving) entities.
    auto group
        = world.registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    for (entt::entity entity : group) {
        auto [sprite, position, previousPos]
            = group.get<Sprite, Position, PreviousPosition>(entity);

        // Get the lerp'd world position.
        Position lerp = MovementHelpers::interpolatePosition(previousPos,
                                                             position, alpha);

        // Update the sprite's world bounds.
        MovementHelpers::moveSpriteWorldBounds(lerp, sprite);
    }
}

void Renderer::renderWorld(const Camera& camera, const double alpha)
{
    // Draw all tiles, layer-by-layer.
    for (unsigned int i = 0; i < world.mapLayers.size(); ++i) {
        for (int y = 0; y < static_cast<int>(WORLD_HEIGHT); ++y) {
            for (int x = 0; x < static_cast<int>(WORLD_WIDTH); ++x) {
                // If there's nothing in this tile, skip it.
                unsigned int linearizedIndex = y * WORLD_WIDTH + x;
                if (world.mapLayers[i][linearizedIndex].texturePtr == nullptr) {
                    continue;
                }

                // Get iso screen extent for this tile.
                Sprite& sprite{world.mapLayers[i][linearizedIndex]};
                SDL2pp::Rect screenExtent = TransformationHelpers::tileToScreenExtent(
                    {x, y}, camera, sprite);

                // If the tile's sprite is within the screen bounds, draw it.
                if (isWithinScreenBounds(screenExtent, camera)) {
                    sdlRenderer.Copy(*(sprite.texturePtr), sprite.texExtent,
                                     screenExtent);
                }
            }
        }
    }

    // Draw all entities with a Sprite, PreviousPosition and Position.
    auto group
        = world.registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    for (entt::entity entity : group) {
        auto [sprite, position, previousPos]
            = group.get<Sprite, Position, PreviousPosition>(entity);

        // Get the lerp'd world position.
        Position lerp = MovementHelpers::interpolatePosition(previousPos,
                                                             position, alpha);

        // Get the iso screen extent for the lerped sprite.
        SDL2pp::Rect screenExtent
            = TransformationHelpers::worldToScreenExtent(lerp, camera, sprite);

        // If the entity's sprite is within the screen bounds, draw it.
        if (isWithinScreenBounds(screenExtent, camera)) {
            sdlRenderer.Copy(*(sprite.texturePtr), sprite.texExtent,
                             screenExtent);
        }
    }
}

void Renderer::renderUserInterface(const Camera& camera)
{
    /* Render the mouse highlight (currently just a tile sprite.) */
    // Get iso screen extent for this tile.
    TileIndex& highlightIndex = ui.tileHighlightIndex;
    Sprite& highlightSprite = ui.tileHighlightSprite;
    SDL2pp::Rect screenExtent = TransformationHelpers::tileToScreenExtent(highlightIndex,
        camera, highlightSprite);

    // Set the texture's alpha to make the highlight transparent.
    highlightSprite.texturePtr->SetAlphaMod(150);

    // Draw the highlight.
    sdlRenderer.Copy(*(highlightSprite.texturePtr), highlightSprite.texExtent,
                     screenExtent);

    // Set the texture's alpha back.
    highlightSprite.texturePtr->SetAlphaMod(255);
}

bool Renderer::isWithinScreenBounds(const SDL2pp::Rect& extent, const Camera& camera)
{
    // The extent is in final screen coordinates, so we only need to check if
    // it's within the rect formed by (0, 0) and (camera.width, camera.height).
    bool pastLeftBound = ((extent.x + extent.w) < 0);
    bool pastRightBound = (camera.extent.width < extent.x);
    bool pastTopBound = ((extent.y + extent.h) < 0);
    bool pastBottomBound = (camera.extent.height < extent.y);

    // If the extent is outside of any camera bound, return false.
    if (pastLeftBound || pastRightBound || pastTopBound || pastBottomBound) {
        return false;
    }
    else {
        // Extent is within the camera bounds, return true.
        return true;
    }
}

void Renderer::drawBoundingBox(const BoundingBox& box, const Camera& camera)
{
    // Transform all the vertices to screen space.
    std::vector<ScreenPoint> verts;
    Position position{box.minX, box.minY, box.minZ};
    verts.push_back(TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.minY, box.minZ};
    verts.push_back(TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.maxY, box.minZ};
    verts.push_back(TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.minX, box.maxY, box.minZ};
    verts.push_back(TransformationHelpers::worldToScreen(position, camera.zoomFactor));

    position = {box.minX, box.minY, box.maxZ};
    verts.push_back(TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.minY, box.maxZ};
    verts.push_back(TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.maxY, box.maxZ};
    verts.push_back(TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.minX, box.maxY, box.maxZ};
    verts.push_back(TransformationHelpers::worldToScreen(position, camera.zoomFactor));

    // Adjust all verts for the camera.
    for (ScreenPoint& vert : verts) {
        vert.x = std::round(vert.x - camera.extent.x);
        vert.y = std::round(vert.y - camera.extent.y);
    }

    // Fill Sint arrays with the verts.
    Sint16 xValues[8] = {};
    Sint16 yValues[8] = {};
    for (unsigned int i = 0; i < 8; ++i) {
        xValues[i] = verts[i].x;
        yValues[i] = verts[i].y;
    }

    // Print the faces.
    filledPolygonRGBA(sdlRenderer.Get(), xValues, yValues, 4, 200, 0, 50, 150);
    filledPolygonRGBA(sdlRenderer.Get(), &(xValues[4]), &(yValues[4]), 4, 255, 0, 0, 150);

    sdlRenderer.SetDrawColor(0, 0, 0, 255);
}

} // namespace Client
} // namespace AM
