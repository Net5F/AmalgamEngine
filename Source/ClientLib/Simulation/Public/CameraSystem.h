#pragma once

namespace AM
{
struct Position;
struct Camera;

namespace Client
{
class World;

/**
 * Moves camera entities according to their behavior.
 */
class CameraSystem
{
public:
    CameraSystem(World& inWorld);

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

    World& world;
};

} // namespace Client
} // namespace AM
