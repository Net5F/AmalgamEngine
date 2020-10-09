#include "IDPool.h"
#include "Log.h"

namespace AM
{
namespace Server
{
IDPool::IDPool()
: IDs{}
{
}

Uint32 IDPool::reserveID()
{
    for (Uint32 i = 0; i < MAX_ENTITIES; ++i) {
        // Find the first false.
        if (!(IDs[i])) {
            IDs[i] = true;
            return i;
        }
    }

    LOG_ERROR("Tried to reserve ID when all were taken.");
    return 0;
}

void IDPool::freeID(Uint32 ID)
{
    if (IDs[ID]) {
        IDs[ID] = false;
    }
    else {
        LOG_ERROR("Tried to free an unused ID.");
    }
}

} // namespace Server
} // namespace AM
