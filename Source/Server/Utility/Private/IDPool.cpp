#include "IDPool.h"
#include <iostream>

std::array<bool, AM::MAX_ENTITIES> AM::IDPool::IDs = {}; // Init to 0;

Uint32 AM::IDPool::reserveID()
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

void AM::IDPool::freeID(Uint32 ID)
{
    if (IDs[ID]) {
        IDs[ID] = false;
    }
    else {
        std::cerr << "Tried to free an unused ID." << std::endl;
    }
}
