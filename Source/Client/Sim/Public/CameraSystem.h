#pragma once

#include "entt/entity/registry.hpp"

namespace AM
{
class Position;

namespace Client
{
class Camera;
class Sim;
class World;

/**
 * Processes player entity update messages and moves the entity appropriately.
 */
class CameraSystem
{
public:
    CameraSystem(Sim& inSim, World& inWorld);

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
     * @param entity  The entity. Used to check if it has a Sprite to account for.
     */
    void centerCameraOnEntity(Camera& camera, Position& position, entt::entity entity);

    Sim& sim;
    World& world;
};

} // namespace Client
} // namespace AM
