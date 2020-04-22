#ifndef IDPOOL_H
#define IDPOOL_H

#include "SharedDefs.h"
#include <SDL_stdinc.h>
#include <array>

namespace AM
{

class IDPool
{
public:
    static Uint32 reserveID();

    static void freeID(Uint32 ID);

private:
    /**
     * If ID 'x' is available, IDs[x] will be true. Else, it will be false.
     */
    static std::array<bool, MAX_ENTITIES> IDs;
};

} // namespace AM

#endif /* IDPOOL_H */
