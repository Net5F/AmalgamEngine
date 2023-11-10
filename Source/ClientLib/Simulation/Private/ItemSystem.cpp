#include "ItemSystem.h"
#include "World.h"
#include "Network.h"
#include "Log.h"

namespace AM
{
namespace Client
{
ItemSystem::ItemSystem(World& inWorld, Network& inNetwork)
: world{inWorld}
, network{inNetwork}
{
}

void ItemSystem::processItemUpdates()
{
}

} // namespace Client
} // namespace AM
