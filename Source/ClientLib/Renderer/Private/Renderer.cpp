#include "Renderer.h"
#include "World.h"
#include "UserInterface.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Sprite.h"
#include "Camera.h"
#include "IRendererExtension.h"
#include "Transforms.h"
#include "ClientTransforms.h"
#include "MovementHelpers.h"
#include "ScreenRect.h"
#include "Log.h"
#include "Ignore.h"

#include <SDL_render.h>
#include <SDL2_gfxPrimitives.h>

namespace AM
{
namespace Client
{
Renderer::Renderer(SDL_Renderer* inSdlRenderer, World& inWorld,
                   UserInterface& inUI,
                   std::function<double(void)> inGetSimTickProgress)
: sdlRenderer{inSdlRenderer}
, world{inWorld}
, ui{inUI}
, getSimTickProgress{inGetSimTickProgress}
, worldSpritePreparer{world.registry, world.tileMap}
, extension{nullptr}
{
}

void Renderer::render()
{
    /* Set up the camera for this frame. */
    // Get how far we are between sim ticks in decimal percent.
    double alpha = getSimTickProgress();

    // Get the lerped camera position based on the alpha.
    auto [playerCamera, playerSprite, playerPosition, playerPreviousPos]
        = world.registry.get<Camera, Sprite, Position, PreviousPosition>(
            world.playerEntity);
    Position cameraLerp{MovementHelpers::interpolatePosition(
        playerCamera.prevPosition, playerCamera.position, alpha)};

    // Set the camera at the lerped position.
    Camera lerpedCamera{playerCamera};
    lerpedCamera.position.x = cameraLerp.x;
    lerpedCamera.position.y = cameraLerp.y;

    // Get iso screen coords for the center point of camera.
    ScreenPoint lerpedCameraCenter{Transforms::worldToScreen(
        lerpedCamera.position, lerpedCamera.zoomFactor)};

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
    SDL_RenderClear(sdlRenderer);

    // Call the project's pre-world-rendering logic.
    if (extension != nullptr) {
        extension->beforeWorld(lerpedCamera, alpha);
    }

    // Draw tiles and entities.
    renderWorld(lerpedCamera, alpha);

    // Call the project's post-world-rendering logic.
    if (extension != nullptr) {
        extension->afterWorld(lerpedCamera, alpha);
    }

    // Draw UI elements.
    ui.render(lerpedCamera);

    // Render the finished buffer to the screen.
    SDL_RenderPresent(sdlRenderer);
}

bool Renderer::handleOSEvent(SDL_Event& event)
{
    // Check if the project wants to handle the event.
    if (extension != nullptr) {
        if (extension->handleOSEvent(event)) {
            return true;
        }
    }

    // The project didn't handle the event. Handle it ourselves.
    switch (event.type) {
        case SDL_WINDOWEVENT:
            // TODO: Handle this.
            return true;
    }

    return false;
}

void Renderer::setExtension(std::unique_ptr<IRendererExtension> inExtension)
{
    extension = std::move(inExtension);
}

void Renderer::renderWorld(const Camera& camera, double alpha)
{
    // Prepare sprites (bounds updating, screen position calcs, depth sorting).
    std::vector<SpriteRenderInfo>& sprites{
        worldSpritePreparer.prepareSprites(camera, alpha)};

    // Draw depth-sorted tiles and sprites.
    // Note: These are already culled during the gather step.
    for (SpriteRenderInfo& spriteInfo : sprites) {
        SDL_RenderCopy(sdlRenderer, spriteInfo.sprite->texture.get(),
                       &(spriteInfo.sprite->textureExtent),
                       &(spriteInfo.screenExtent));
        //        drawBoundingBox(spriteInfo.worldBounds, camera);
    }
}

void Renderer::drawBoundingBox(const BoundingBox& box, const Camera& camera)
{
    // Transform all the vertices to screen space.
    std::vector<ScreenPoint> verts;
    Position position{box.minX, box.minY, box.minZ};
    verts.push_back(Transforms::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.minY, box.minZ};
    verts.push_back(Transforms::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.maxY, box.minZ};
    verts.push_back(Transforms::worldToScreen(position, camera.zoomFactor));
    position = {box.minX, box.maxY, box.minZ};
    verts.push_back(Transforms::worldToScreen(position, camera.zoomFactor));

    position = {box.minX, box.minY, box.maxZ};
    verts.push_back(Transforms::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.minY, box.maxZ};
    verts.push_back(Transforms::worldToScreen(position, camera.zoomFactor));
    position = {box.maxX, box.maxY, box.maxZ};
    verts.push_back(Transforms::worldToScreen(position, camera.zoomFactor));
    position = {box.minX, box.maxY, box.maxZ};
    verts.push_back(Transforms::worldToScreen(position, camera.zoomFactor));

    // Adjust all verts for the camera.
    for (ScreenPoint& vert : verts) {
        vert.x = std::round(vert.x - camera.extent.x);
        vert.y = std::round(vert.y - camera.extent.y);
    }

    // Fill Sint arrays with the verts.
    Sint16 xValues[8] = {};
    Sint16 yValues[8] = {};
    for (unsigned int i = 0; i < 8; ++i) {
        xValues[i] = static_cast<Sint16>(verts[i].x);
        yValues[i] = static_cast<Sint16>(verts[i].y);
    }

    // Draw the faces.
    filledPolygonRGBA(sdlRenderer, xValues, yValues, 4, 200, 0, 50, 150);
    filledPolygonRGBA(sdlRenderer, &(xValues[4]), &(yValues[4]), 4, 255, 0, 0,
                      150);

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
}

} // namespace Client
} // namespace AM
