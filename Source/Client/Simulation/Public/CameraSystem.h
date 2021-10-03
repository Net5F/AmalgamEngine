#pragma once

#include "entt/entity/registry.hpp"

namespace AM
{
class Position;
class Camera;

namespace Client
{
class Simulation;
class World;

/**
 * Moves camera entities according to their behavior.
 */
class CameraSystem
{
public:
    CameraSystem(Simulation& inSim, World& inWorld);

    /**
     * Moves all cameras to their appropriate next positions.
     */
    void moveCameras();

private:
    /**
     * Centers the camera on the given position.
     * If the given entity has a sprite, offsets the position to center on it.
     * @param camera  The camera to move.
     * @param position  The entity's Position.
     */
    void moveCameraToPosition(Camera& camera, Position& position);

    Simulation& sim;
    World& world;
};

} // namespace Client
} // namespace AM
