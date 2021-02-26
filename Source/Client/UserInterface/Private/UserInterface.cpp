#include "UserInterface.h"
#include "World.h"
#include "Camera.h"
#include "SimDefs.h"
#include "Log.h"

namespace AM
{
namespace Client
{

UserInterface::UserInterface(World& inWorld)
: world(inWorld)
{
}

bool UserInterface::handleEvent(SDL_Event& event)
{
    switch (event.type) {
        case SDL_MOUSEMOTION:
            handleMouseMotion(event.motion);

            // Temporarily consuming mouse events until the sim has some use
            // for them.
            return true;
    }

    return false;
}

void UserInterface::handleMouseMotion(SDL_MouseMotionEvent& event)
{
    // Account for the camera screen position and zoom factor.
    Camera& playerCamera = world.registry.get<Camera>(world.playerEntity);
    float screenX = (event.x + playerCamera.extent.x) / playerCamera.zoomFactor;
    float screenY = (event.y + playerCamera.extent.y) / playerCamera.zoomFactor;

    // Remove the half-tile X offset that we add to align tile sprites with
    // non-tile sprites.
    screenX -= (TILE_SCREEN_WIDTH / 2.f);

    // Calc the scaling factor going from screen tiles to world tiles.
    static const float TILE_WIDTH_SCALE
        = static_cast<float>(TILE_WORLD_WIDTH) / TILE_SCREEN_WIDTH;
    static const float TILE_HEIGHT_SCALE
        = static_cast<float>(TILE_WORLD_HEIGHT) / TILE_SCREEN_HEIGHT;

    // Calc the world position.
    float worldX = ((2 * screenY) + screenX) * (TILE_WIDTH_SCALE);
    float worldY = ((2 * screenY) - screenX) * (TILE_HEIGHT_SCALE) / 2;

    LOG_INFO("Mouse at screen: (%d, %d), world: (%.4f, %.4f)", event.x, event.y,
             worldX, worldY);
}

} // End namespace Client
} // End namespace AM
