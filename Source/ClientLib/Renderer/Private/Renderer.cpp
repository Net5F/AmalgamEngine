#include "Renderer.h"
#include "World.h"
#include "UserInterface.h"
#include "GraphicData.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Sprite.h"
#include "Camera.h"
#include "IRendererExtension.h"
#include "Transforms.h"
#include "MovementHelpers.h"
#include "Log.h"
#include <SDL_render.h>
#include <SDL2_gfxPrimitives.h>

namespace AM
{
namespace Client
{
Renderer::Renderer(SDL_Renderer* inSdlRenderer, World& inWorld,
                   UserInterface& inUI, GraphicData& inGraphicData,
                   std::function<double(void)> inGetSimTickProgress)
: sdlRenderer{inSdlRenderer}
, world{inWorld}
, ui{inUI}
, graphicData{inGraphicData}
, getSimTickProgress{inGetSimTickProgress}
, worldSpriteSorter{world.registry, world.tileMap, graphicData, ui}
, extension{nullptr}
{
}

void Renderer::render()
{
    /* Set up the camera for this frame. */
    // Get how far we are between sim ticks in decimal percent.
    double alpha{getSimTickProgress()};

    // If we have a valid player entity, get the lerped camera.
    // Note: When we're disconnected from the server, the camera will be
    //       default (the UI will be covering it anyway).
    Camera lerpedCamera{};
    if (world.playerEntity != entt::null) {
        lerpedCamera = getLerpedCamera(alpha);
    }

    /* Render. */
    // Clear the screen to prepare for drawing.
    SDL_RenderClear(sdlRenderer);

    // Call the project's pre-world-rendering logic.
    if (extension != nullptr) {
        extension->beforeWorld(lerpedCamera, alpha);
    }

    // Draw tiles and entities.
    // Note: As part of this, the sorter's sorted sprites vector is updated.
    renderWorld(lerpedCamera, alpha);

    // Call the project's post-world-rendering logic.
    if (extension != nullptr) {
        extension->afterWorld(lerpedCamera, alpha);
    }

    // Draw UI elements.
    // Note: We pass the sorted sprites list so the UI can update its locator.
    ui.render(lerpedCamera, worldSpriteSorter.getSortedSprites());

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

Camera Renderer::getLerpedCamera(double alpha)
{
    // Get the lerped camera position based on the alpha.
    auto [playerCamera, playerPosition, playerPreviousPos]
        = world.registry.get<Camera, Position, PreviousPosition>(
            world.playerEntity);
    Position cameraLerp{MovementHelpers::interpolatePosition(
        playerCamera.prevPosition, playerCamera.position, alpha)};

    // Set the camera at the lerped position.
    Camera lerpedCamera{playerCamera};
    lerpedCamera.position.x = cameraLerp.x;
    lerpedCamera.position.y = cameraLerp.y;

    // Get iso screen coords for the center point of camera.
    SDL_FPoint lerpedCameraCenter{Transforms::worldToScreen(
        lerpedCamera.position, lerpedCamera.zoomFactor)};

    // Calc where the top left of the lerpedCamera is in screen space.
    lerpedCamera.extent.x = lerpedCameraCenter.x - (lerpedCamera.extent.w / 2);
    lerpedCamera.extent.y = lerpedCameraCenter.y - (lerpedCamera.extent.h / 2);

    // Save the last calculated screen position of the camera for use in
    // screen -> world calcs.
    playerCamera.extent.x = lerpedCamera.extent.x;
    playerCamera.extent.y = lerpedCamera.extent.y;

    return lerpedCamera;
}

void Renderer::renderWorld(const Camera& camera, double alpha)
{
    // Prepare sprites (bounds updating, screen position calcs, depth sorting).
    worldSpriteSorter.sortSprites(camera, alpha);
    const std::vector<SpriteSortInfo>& sortedSprites{
        worldSpriteSorter.getSortedSprites()};

    // Draw depth-sorted tiles and sprites.
    // Note: These are already culled during the gather step.
    for (const SpriteSortInfo& spriteInfo : sortedSprites) {
        const SpriteRenderData& renderData{
            graphicData.getRenderData(spriteInfo.sprite->numericID)};
        const SDL_Color& colorMod{spriteInfo.colorMod};

        // Apply the alpha mod that the UI gave us.
        SDL_SetTextureAlphaMod(renderData.texture.get(), colorMod.a);

        // Render the sprite.
        SDL_RenderCopyF(sdlRenderer, renderData.texture.get(),
                        &(renderData.textureExtent), &(spriteInfo.screenExtent));

        // If the UI gave us a color mod to apply, render an additional sprite
        // with an additive blend mode and apply the color to that.
        if (colorMod.r > 0 || colorMod.g > 0 || colorMod.b > 0) {
            SDL_SetTextureColorMod(renderData.texture.get(), colorMod.r,
                                   colorMod.g, colorMod.b);
            SDL_SetTextureBlendMode(renderData.texture.get(),
                                    SDL_BLENDMODE_ADD);

            SDL_RenderCopyF(sdlRenderer, renderData.texture.get(),
                            &(renderData.textureExtent),
                            &(spriteInfo.screenExtent));

            SDL_SetTextureBlendMode(renderData.texture.get(),
                                    SDL_BLENDMODE_BLEND);
            SDL_SetTextureColorMod(renderData.texture.get(), 255, 255, 255);
        }

        // Reset the texture's alpha.
        SDL_SetTextureAlphaMod(renderData.texture.get(), 255);

        //        drawBoundingBox(spriteInfo.worldBounds, camera);
    }
}

void Renderer::drawBoundingBox(const BoundingBox& box, const Camera& camera)
{
    // Transform all the vertices to screen space.
    std::vector<SDL_FPoint> verts;
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
    for (SDL_FPoint& vert : verts) {
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
