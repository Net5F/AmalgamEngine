#include "UserInterface.h"
#include "World.h"
#include "AssetCache.h"
#include "Camera.h"
#include "Paths.h"
#include "SharedConfig.h"
#include "TransformationHelpers.h"
#include "Log.h"

namespace AM
{
namespace Client
{
UserInterface::UserInterface(World& inWorld, AssetCache& inAssetCache)
: tileHighlightSprite{}
, tileHighlightIndex{0, 0}
, world{inWorld}
, assetCache{inAssetCache}
{
    // Set up the tile highlight sprite.
    TextureHandle texture = assetCache.loadTexture(Paths::TEXTURE_DIR + "iso_test_sprites.png");
    tileHighlightSprite
        = {texture, {(256 * 8), (512 * 0), 256, 512}, 256, 512};

    // Push the terrain sprites to cycle through.
    SDL2pp::Rect spritePosInTexture{(256 * 6), (512 * 0), 256, 512};
    terrainSprites.push_back(
        Sprite{texture, spritePosInTexture, 256, 512});

    spritePosInTexture = {(256 * 6), (512 * 2), 256, 512};
    terrainSprites.push_back(
        Sprite{texture, spritePosInTexture, 256, 512});

    spritePosInTexture = {(256 * 6), (512 * 3), 256, 512};
    terrainSprites.push_back(
        Sprite{texture, spritePosInTexture, 256, 512});
}

bool UserInterface::handleEvent(SDL_Event& event)
{
    switch (event.type) {
        // TODO: If the player moves through key presses but doesn't move the
        //       mouse, the highlight won't update. Decide how to handle it.
        case SDL_MOUSEMOTION:
            handleMouseMotion(event.motion);

            // Temporarily consuming mouse events until the sim has some use
            // for them.
            return true;
        case SDL_MOUSEBUTTONDOWN:
            handleMouseButtonDown(event.button);
            return true;
    }

    return false;
}

void UserInterface::handleMouseMotion(SDL_MouseMotionEvent& event)
{
    // Get the tile index that the mouse is hovering over.
    Camera& playerCamera = world.registry.get<Camera>(world.playerEntity);
    ScreenPoint screenPoint{static_cast<float>(event.x),
                            static_cast<float>(event.y)};
    TileIndex tileIndex = TransformationHelpers::screenToTile(screenPoint, playerCamera);

    // If the index is outside of the world bounds, ignore this event.
    if ((tileIndex.x < 0)
        || (tileIndex.y < 0)
        || (tileIndex.x >= static_cast<int>(SharedConfig::WORLD_WIDTH))
        || (tileIndex.y >= static_cast<int>(SharedConfig::WORLD_HEIGHT))) {
        return;
    }

    // Save the new index for the renderer to use.
    tileHighlightIndex
        = TransformationHelpers::screenToTile(screenPoint, playerCamera);
}

void UserInterface::handleMouseButtonDown(SDL_MouseButtonEvent& event)
{
    switch (event.button) {
        case SDL_BUTTON_LEFT:
            cycleTile(event.x, event.y);
            break;
    }
}

void UserInterface::cycleTile(int mouseX, int mouseY)
{
    // Find the tile index under the mouse's current position.
    Camera& playerCamera = world.registry.get<Camera>(world.playerEntity);
    ScreenPoint screenPoint{static_cast<float>(mouseX),
                            static_cast<float>(mouseY)};
    TileIndex tileIndex
        = TransformationHelpers::screenToTile(screenPoint, playerCamera);

    // If the index is outside of the world bounds, ignore this event.
    if ((tileIndex.x < 0)
        || (tileIndex.y < 0)
        || (tileIndex.x >= static_cast<int>(SharedConfig::WORLD_WIDTH))
        || (tileIndex.y >= static_cast<int>(SharedConfig::WORLD_HEIGHT))) {
        return;
    }

    // Determine which sprite the selected tile has.
    unsigned int linearizedIndex = tileIndex.y * SharedConfig::WORLD_WIDTH + tileIndex.x;
    Sprite& tileSprite = world.mapLayers[0][linearizedIndex];

    unsigned int terrainSpriteIndex = 0;
    switch (tileSprite.textureExtent.y) {
        case (512 * 0):
            terrainSpriteIndex = 0;
            break;
        case (512 * 2):
            terrainSpriteIndex = 1;
            break;
        case (512 * 3):
            terrainSpriteIndex = 2;
            break;
    }

    // Set the tile to the next sprite.
    terrainSpriteIndex++;
    terrainSpriteIndex %= 3;
    world.mapLayers[0][linearizedIndex] = terrainSprites[terrainSpriteIndex];
}

} // End namespace Client
} // End namespace AM
