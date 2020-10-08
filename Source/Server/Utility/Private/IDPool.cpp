#include "IDPool.h"
#include "Debug.h"

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

    DebugError("Tried to reserve ID when all were taken.");
    return 0;
}

void IDPool::freeID(Uint32 ID)
{
    if (IDs[ID]) {
        IDs[ID] = false;
    }
    else {
        DebugError("Tried to free an unused ID.");
    }
}

} // namespace Server
} // namespace AM
