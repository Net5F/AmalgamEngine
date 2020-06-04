#include "IDPool.h"
#include <iostream>

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
    for (Uint16 i = 0; i < MAX_ENTITIES; ++i) {
        // Find the first false.
        if (!(IDs[i])) {
            IDs[i] = true;
            return i;
        }
    }

    std::cerr << "Tried to reserve ID when all were taken. Returning 0." << std::endl;
    return 0;
}

void IDPool::freeID(Uint32 ID)
{
    if (IDs[ID]) {
        IDs[ID] = false;
    }
    else {
        std::cerr << "Tried to free an unused ID." << std::endl;
    }
}

} // namespace Server
} // namespace AM
