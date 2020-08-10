#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "GameDefs.h"
#include "InputComponent.h"
#include "Message_generated.h"
#include <array>

namespace AM
{
namespace Server
{

class World;

class MovementSystem
{
public:
    MovementSystem(World& inWorld);

    /**
     * Updates movement components based on input state, moves position components
     * based on movement, updates sprites based on position.
     */
    void processMovements(Uint64 deltaCount);

private:
    World& world;
};

} // namespace Server
} // namespace AM

#endif /* MOVEMENTSYSTEM_H */
