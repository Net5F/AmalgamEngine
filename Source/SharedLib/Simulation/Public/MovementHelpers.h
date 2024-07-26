#pragma once

#include "Input.h"
#include "Position.h"
#include "Vector3.h"
#include "Rotation.h"
#include "SharedConfig.h"

namespace AM
{
struct Position;
struct PreviousPosition;
struct Movement;
struct MovementModifiers;
struct Collision;
class TileMapBase;
class EntityLocator;

/**
 * Shared static functions for moving entities.
 */
class MovementHelpers
{
public:
    /**
     * Calculates an updated velocity based on the given data.

     * @param inputStates The current input state.
     * @param[out] movement The current movement state (may update velocity 
     *                      and jumpCount).
     * @param movementMods The current movement mod state.
     *
     * @return The updated velocity.
     */
    static Vector3 calcVelocity(const Input::StateArr& inputStates,
                                Movement& movement,
                                const MovementModifiers& movementMods);

    /**
     * Calculates an updated position based on the given data.
     *
     * @param position The current position.
     * @param velocity The current velocity.
     * @param deltaSeconds The number of seconds that have passed since the
     *                     last update.
     *
     * @return The updated position.
     */
    static Position calcPosition(const Position& position,
                                 const Vector3& velocity, double deltaSeconds);

    /**
     * Calculates a rotation based on the given input state.
     * If there are no inputs or they cancel out, the given rotation will be
     * returned.
     *
     * @param rotation The current rotation.
     * @param inputStates The current input state.
     */
    static Rotation calcRotation(const Rotation& rotation,
                                 const Input::StateArr& inputStates);

    /**
     * Returns a position interpolated between previousPos and position.
     */
    static Position interpolatePosition(const PreviousPosition& previousPos,
                                        const Position& position, double alpha);

    /** An epsilon that can be used when comparing float world positions to 
        integer values, to account for float precision loss.
        Calculated by finding the float precision at the furthest position of 
        the largest map that we support.
        Reference: https://blog.demofox.org/2017/11/21/floating-point-precision/
        Note: The division by 2 is because we center the map on the origin. */
    static constexpr float MAX_WORLD_VALUE{SharedConfig::MAX_MAP_WIDTH_TILES
                                           * SharedConfig::TILE_WORLD_WIDTH
                                           / 2.f};
    static constexpr float WORLD_EPSILON{MAX_WORLD_VALUE
                                         / static_cast<float>(1 << 23)};

private:
    /**
     * Returns the appropriate direction for the given direction int.
     * @param directionInt An integer representation of a direction, derived
     *                     from the formula (3 * (yDown - yUp) + xUp - xDown).
     */
    static Rotation::Direction directionIntToDirection(int directionInt);

    /**
     * Performs the first step of velocity calculations, assuming the entity 
     * can fly.
     */
    static Vector3 calcVelocityCanFly(const Input::StateArr& inputStates,
                                      Movement& movement,
                                      const MovementModifiers& movementMods);

    /**
     * Performs the first step of velocity calculations, assuming the entity 
     * cannot fly.
     */
    static Vector3 calcVelocityNoFly(const Input::StateArr& inputStates,
                                     Movement& movement,
                                     const MovementModifiers& movementMods);
};

} // End namespace AM
