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
    World& world;
};

} // namespace Client
} // namespace AM
