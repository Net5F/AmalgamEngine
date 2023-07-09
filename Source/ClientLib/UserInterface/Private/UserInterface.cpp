#include "UserInterface.h"
#include "World.h"
#include "Config.h"
#include "AssetCache.h"
#include "SpriteData.h"
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

UserInterface::UserInterface()
: eventDispatcher{}
, extension{nullptr}
, worldObjectLocator{}
{
}

std::vector<PhantomSpriteInfo>
    UserInterface::getPhantomSprites() const
{
    // Get the project's phantom tiles.
    if (extension != nullptr) {
        return extension->getPhantomSprites();
    }
    else {
        return {};
    }
}

std::vector<SpriteColorModInfo>
    UserInterface::getSpriteColorMods() const
{
    // Get the project's tile color mods.
    if (extension != nullptr) {
        return extension->getSpriteColorMods();
    }
    else {
        return {};
    }
}

void UserInterface::tick(double timestepS)
{
    // Call the project's UI tick logic.
    if (extension != nullptr) {
        extension->tick(timestepS);
    }
}

void UserInterface::render(const Camera& camera,
                const std::vector<SpriteSortInfo>& sortedSprites)
{
    // Call the project's UI rendering logic.
    if (extension != nullptr) {
        extension->render(camera);
    }

    // Update our locator's data and fill it with the latest sprites.
    worldObjectLocator.setCamera(camera);
    worldObjectLocator.setExtent(
        camera.getTileViewExtent(world->tileMap.getTileExtent()));

    worldObjectLocator.clear();
    for (const SpriteSortInfo& spriteInfo : sortedSprites) {
        // If this sprite isn't a full phantom, add it.
        if (!std::get_if<std::monostate>(&(spriteInfo.spriteOwnerID))) {
            worldObjectLocator.addWorldObject(spriteInfo.spriteOwnerID,
                                              spriteInfo.worldBounds);
        }
    }
}

bool UserInterface::handleOSEvent(SDL_Event& event)
{
    // Check if the project wants to handle the event.
    if (extension != nullptr) {
        return extension->handleOSEvent(event);
    }

    return false;
}

const WorldObjectLocator& UserInterface::getWorldObjectLocator()
{
    return worldObjectLocator;
}

EventDispatcher& UserInterface::getEventDispatcher()
{
    return eventDispatcher;
}

void UserInterface::setExtension(
    std::unique_ptr<IUserInterfaceExtension> inExtension)
{
    extension = std::move(inExtension);
}

void UserInterface::setWorld(const World& inWorld)
{
    world = &inWorld;
}

} // End namespace Client
} // End namespace AM
