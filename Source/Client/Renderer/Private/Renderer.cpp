#include "Renderer.h"
#include "Simulation.h"
#include "UserInterface.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Sprite.h"
#include "Camera.h"
#include "TransformationHelpers.h"
#include "MovementHelpers.h"
#include "ScreenRect.h"
#include "Log.h"
#include "Ignore.h"

#include <SDL2/SDL2_gfxPrimitives.h>

namespace AM
{
namespace Client
{
Renderer::Renderer(SDL2pp::Renderer& inSdlRenderer, SDL2pp::Window& window,
                   Simulation& inSim, UserInterface& inUI,
                   std::function<double(void)> inGetProgress)
: sdlRenderer(inSdlRenderer)
, sim(inSim)
, world(sim.getWorld())
, ui(inUI)
, getProgress(inGetProgress)
, worldSpritePreparer(world.registry, world.mapLayers)
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
    ScreenPoint lerpedCameraCenter = TransformationHelpers::worldToScreen(
        lerpedCamera.position, lerpedCamera.zoomFactor);

    // Calc where the top left of the lerpedCamera is in screen space.
    lerpedCamera.extent.x
        = lerpedCameraCenter.x - (lerpedCamera.extent.width / 2);
    lerpedCamera.extent.y
        = lerpedCameraCenter.y - (lerpedCamera.extent.height / 2);

    // Save the last calculated screen position of the camera for use in
    // screen -> world calcs.
    playerCamera.extent.x = lerpedCamera.extent.x;
    playerCamera.extent.y = lerpedCamera.extent.y;

    /* Render. */
    // Clear the screen to prepare for drawing.
    sdlRenderer.Clear();

    // Draw tiles and entities.
    renderWorld(lerpedCamera, alpha);

    // Draw UI elements.
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

void Renderer::renderWorld(const Camera& camera, double alpha)
{
    // Prepare sprites (bounds updating, screen position calcs, depth sorting).
    std::vector<SpriteRenderInfo>& sprites
        = worldSpritePreparer.prepareSprites(camera, alpha);

    // Draw depth-sorted tiles and sprites.
    // Note: These are already culled during the gather step.
    for (SpriteRenderInfo& spriteInfo : sprites) {
        sdlRenderer.Copy(spriteInfo.sprite->textureHandle.get(),
                         spriteInfo.sprite->textureExtent,
                         spriteInfo.screenExtent);
        drawBoundingBox(spriteInfo.sprite->worldBounds, camera);
    }
}

void Renderer::renderUserInterface(const Camera& camera)
{
    /* Render the mouse highlight (currently just a tile sprite.) */
    // Get iso screen extent for this tile.
    TileIndex& highlightIndex = ui.tileHighlightIndex;
    Sprite& highlightSprite = ui.tileHighlightSprite;
    SDL2pp::Rect screenExtent = TransformationHelpers::tileToScreenExtent(
        highlightIndex, camera, highlightSprite);

    // Set the texture's alpha to make the highlight transparent.
    highlightSprite.textureHandle.get().SetAlphaMod(150);

    // Draw the highlight.
    sdlRenderer.Copy(highlightSprite.textureHandle,
                     highlightSprite.textureExtent, screenExtent);

    // Set the texture's alpha back.
    highlightSprite.textureHandle.get().SetAlphaMod(255);
}

void Renderer::drawBoundingBox(const BoundingBox& box, const Camera& camera)
{
    // Transform all the vertices to screen space.
    std::vector<ScreenPoint> verts;
    Position position{box.minX, box.minY, box.minZ};
    verts.push_back(
        TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.minY, box.minZ};
    verts.push_back(
        TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.maxY, box.minZ};
    verts.push_back(
        TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.minX, box.maxY, box.minZ};
    verts.push_back(
        TransformationHelpers::worldToScreen(position, camera.zoomFactor));

    position = {box.minX, box.minY, box.maxZ};
    verts.push_back(
        TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.minY, box.maxZ};
    verts.push_back(
        TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.maxY, box.maxZ};
    verts.push_back(
        TransformationHelpers::worldToScreen(position, camera.zoomFactor));
    position = {box.minX, box.maxY, box.maxZ};
    verts.push_back(
        TransformationHelpers::worldToScreen(position, camera.zoomFactor));

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

    // Draw the faces.
    filledPolygonRGBA(sdlRenderer.Get(), xValues, yValues, 4, 200, 0, 50, 150);
    filledPolygonRGBA(sdlRenderer.Get(), &(xValues[4]), &(yValues[4]), 4, 255,
                      0, 0, 150);

    sdlRenderer.SetDrawColor(0, 0, 0, 255);
}

} // namespace Client
} // namespace AM
