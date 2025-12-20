#pragma once

namespace AM
{
struct Position;
struct Camera;

namespace Client
{
struct SimulationContext;
class World;

/**
 * Moves camera entities according to their behavior.
 */
class CameraSystem
{
public:
    CameraSystem(const SimulationContext& inSimContext);

    /**
     * Moves all cameras to their appropriate next positions.
     */
    void moveCameras();

private:
    World& world;
};

} // namespace Client
} // namespace AM
