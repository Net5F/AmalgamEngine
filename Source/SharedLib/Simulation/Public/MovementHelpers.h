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
     * Updates the given movement state based on the given data.

     * @param inputStates The current input state.
     * @param movementMods The current movement mod state.
     * @param[out] movement The current movement state.
     *
     * @post movement is updated.
     */
    static void updateMovement(const Input::StateArr& inputStates,
                               const MovementModifiers& movementMods,
                               Movement& movement);

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
     * Note: Won't change movement.velocity. May change other fields.
     */
    static Vector3 calcVelocityCanFly(const Input::StateArr& inputStates,
                                      const MovementModifiers& movementMods,
                                      Movement& movement);

    /**
     * Performs the first step of velocity calculations, assuming the entity
     * cannot fly.
     * Note: Won't change movement.velocity. May change other fields.
     */
    static Vector3 calcVelocityNoFly(const Input::StateArr& inputStates,
                                     const MovementModifiers& movementMods,
                                     Movement& movement);
};

} // End namespace AM
