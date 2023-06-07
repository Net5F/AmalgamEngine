#include "UserInterface.h"
#include "Config.h"
#include "AssetCache.h"
#include "SpriteData.h"
#include "Camera.h"
#include "IUserInterfaceExtension.h"
#include "SharedConfig.h"
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
, phantomTileSprites{}
, tileSpriteColorMods{}
{
}

std::vector<PhantomTileSpriteInfo>
    UserInterface::getPhantomTileSprites() const
{
    // Get the project's phantom tiles.
    if (extension != nullptr) {
        return extension->getPhantomTileSprites();
    }
    else {
        return {};
    }
}

std::vector<TileSpriteColorModInfo>
    UserInterface::getTileSpriteColorMods() const
{
    // Get the project's tile color mods.
    if (extension != nullptr) {
        return extension->getTileSpriteColorMods();
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

void UserInterface::render(const Camera& camera)
{
    // Call the project's UI rendering logic.
    if (extension != nullptr) {
        extension->render(camera);
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

EventDispatcher& UserInterface::getEventDispatcher()
{
    return eventDispatcher;
}

void UserInterface::setExtension(
    std::unique_ptr<IUserInterfaceExtension> inExtension)
{
    extension = std::move(inExtension);
}

} // End namespace Client
} // End namespace AM
