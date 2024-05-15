#include "CameraSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Camera.h"
#include "Position.h"
#include "Log.h"

namespace AM
{
namespace Client
{
CameraSystem::CameraSystem(World& inWorld)
: world{inWorld}
{
}

void CameraSystem::moveCameras()
{
    auto cameraGroup = world.registry.group<Camera>(entt::get<Position>);
    for (auto [entity, camera, position] : cameraGroup.each()) {
        // Save the camera's previous target position.
        camera.prevTarget = camera.target;

        // Update the camera to its new target position based on its behavior.
        switch (camera.behavior) {
            case Camera::Fixed:
                // Doesn't move on its own.
                break;
            case Camera::CenterOnEntity:
                camera.target = position;
                break;
            default:
                break;
        }
    }
}

} // namespace Client
} // namespace AM
