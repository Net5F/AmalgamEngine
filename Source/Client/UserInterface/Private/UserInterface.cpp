#include "UserInterface.h"
#include "QueuedEvents.h"
#include "World.h"
#include "SpriteData.h"
#include "Camera.h"
#include "Paths.h"
#include "SharedConfig.h"
#include "Transforms.h"
#include "TileUpdateRequest.h"
#include "Log.h"

namespace AM
{
namespace Client
{
UserInterface::UserInterface(EventDispatcher& inUiEventDispatcher,
                             const World& inWorld, SpriteData& spriteData)
: tileHighlightSprite{}
, tileHighlightPosition{0, 0}
, uiEventDispatcher{inUiEventDispatcher}
, world{inWorld}
{
    // Set up the tile highlight sprite.
    tileHighlightSprite = &(spriteData.get("test_8"));

    // Push the terrain sprites to cycle through.
    terrainSprites.push_back(&(spriteData.get("test_6")));
    terrainSprites.push_back(&(spriteData.get("test_8")));
    terrainSprites.push_back(&(spriteData.get("test_24")));
    terrainSprites.push_back(&(spriteData.get("test_17")));
    terrainSprites.push_back(&(spriteData.get("test_26")));
}

bool UserInterface::handleOSEvent(SDL_Event& event)
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
    const Camera& playerCamera{world.registry.get<Camera>(world.playerEntity)};
    ScreenPoint screenPoint{static_cast<float>(event.x),
                            static_cast<float>(event.y)};
    TilePosition tilePosition{
        Transforms::screenToTile(screenPoint, playerCamera)};

    // If the index is outside of the world bounds, ignore this event.
    const TileExtent& mapTileExtent{world.tileMap.getTileExtent()};
    if ((tilePosition.x < 0) || (tilePosition.y < 0)
        || (tilePosition.x >= mapTileExtent.xLength)
        || (tilePosition.y >= mapTileExtent.yLength)) {
        return;
    }

    // Save the new tile position for the renderer to use.
    tileHighlightPosition = Transforms::screenToTile(screenPoint, playerCamera);
}

void UserInterface::handleMouseButtonDown(SDL_MouseButtonEvent& event)
{
    switch (event.button) {
        case SDL_BUTTON_LEFT: {
            cycleTile(event.x, event.y);
            break;
        }
    }
}

void UserInterface::cycleTile(int mouseX, int mouseY)
{
    // Find the tile index under the mouse's current position.
    const Camera& playerCamera{world.registry.get<Camera>(world.playerEntity)};
    ScreenPoint screenPoint{static_cast<float>(mouseX),
                            static_cast<float>(mouseY)};
    TilePosition tilePosition{
        Transforms::screenToTile(screenPoint, playerCamera)};

    // If the index is outside of the world bounds, ignore this event.
    const TileExtent& mapTileExtent{world.tileMap.getTileExtent()};
    if ((tilePosition.x < 0) || (tilePosition.y < 0)
        || (tilePosition.x >= mapTileExtent.xLength)
        || (tilePosition.y >= mapTileExtent.yLength)) {
        return;
    }

    // Determine which sprite the selected tile has.
    const Tile& tile{world.tileMap.getTile(tilePosition.x, tilePosition.y)};

    unsigned int terrainSpriteIndex{0};
    if (tile.spriteLayers[0].sprite->stringID == "test_6") {
        terrainSpriteIndex = 0;
    }
    else if (tile.spriteLayers[0].sprite->stringID == "test_8") {
        terrainSpriteIndex = 1;
    }
    else if (tile.spriteLayers[0].sprite->stringID == "test_24") {
        terrainSpriteIndex = 2;
    }
    else if (tile.spriteLayers[0].sprite->stringID == "test_17") {
        terrainSpriteIndex = 3;
    }
    else if (tile.spriteLayers[0].sprite->stringID == "test_26") {
        terrainSpriteIndex = 4;
    }

    // Set the tile to the next sprite.
    terrainSpriteIndex++;
    terrainSpriteIndex %= terrainSprites.size();
    uiEventDispatcher.emplace<TileUpdateRequest>(
        tilePosition.x, tilePosition.y, 0,
        terrainSprites[terrainSpriteIndex]->numericID);
}

} // End namespace Client
} // End namespace AM
