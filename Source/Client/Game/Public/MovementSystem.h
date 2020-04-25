#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "SharedDefs.h"
#include "InputComponent.h"
#include <array>

namespace AM
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
    void processMovements(double deltaMs);

private:
    /**
     * Updates the entity's velocity based on the current input states and delta time.
     */
    void changeVelocity(
    EntityID entityID,
    std::array<Input::State, static_cast<int>(Input::Type::NumTypes)>& inputStates,
    double deltaMs);

    World& world;
};

} // namespace AM

#endif /* MOVEMENTSYSTEM_H */
