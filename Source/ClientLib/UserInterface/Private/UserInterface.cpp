#include "UserInterface.h"
#include "UserInterfaceContext.h"
#include "Simulation.h"
#include "Config.h"
#include "Camera.h"
#include "IUserInterfaceExtension.h"
#include "SharedConfig.h"
#include "UserConfig.h"
#include "Transforms.h"
#include "ClientTransforms.h"
#include "Log.h"

namespace AM
{
namespace Client
{

UserInterface::UserInterface(const UserInterfaceContext& inUIContext)
: world{inUIContext.simulation.getWorld()}
, extension{nullptr}
, worldObjectLocator{}
{
}

std::vector<PhantomSpriteInfo> UserInterface::getPhantomSprites() const
{
    // Get the project's phantom tiles.
    return extension->getPhantomSprites();
}

std::vector<SpriteColorModInfo> UserInterface::getSpriteColorMods() const
{
    // Get the project's tile color mods.
    return extension->getSpriteColorMods();
}

void UserInterface::tick(double timestepS)
{
    // Call the project's UI tick logic.
    extension->tick(timestepS);
}

void UserInterface::render(const Camera& camera,
                           const std::vector<SpriteSortInfo>& sortedSprites)
{
    // Call the project's UI rendering logic.
    extension->render(camera);

    // Update our locator's data and fill it with the latest sprites.
    worldObjectLocator.setCamera(camera);
    worldObjectLocator.setExtent(
        camera.getTileViewExtent(world.tileMap.getTileExtent()));

    worldObjectLocator.clear();
    for (const SpriteSortInfo& spriteInfo : sortedSprites) {
        // If this sprite isn't a full phantom or an A/V entity, add it.
        if (!std::get_if<std::monostate>(&(spriteInfo.spriteOwnerID))
            && !std::get_if<AVEntityID>(&(spriteInfo.spriteOwnerID))) {
            worldObjectLocator.addWorldObject(spriteInfo.spriteOwnerID,
                                              spriteInfo.worldBounds);
        }
    }
}

bool UserInterface::handleOSEvent(SDL_Event& event)
{
    // Let the project handle it.
    return extension->handleOSEvent(event);
}

const WorldObjectLocator& UserInterface::getWorldObjectLocator()
{
    return worldObjectLocator;
}

void UserInterface::setExtension(IUserInterfaceExtension* inExtension)
{
    extension = inExtension;
}

} // End namespace Client
} // End namespace AM
