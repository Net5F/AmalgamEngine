#include "CameraSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Camera.h"
#include "Position.h"
#include "Ignore.h"
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
    for (entt::entity entity : cameraGroup) {
        // Save the camera's previous position.
        auto [camera, position] = cameraGroup.get<Camera, Position>(entity);
        camera.prevPosition.x = camera.position.x;
        camera.prevPosition.y = camera.position.y;

        // Update the camera to its new position based on its behavior.
        switch (camera.behavior) {
            case Camera::Fixed:
                // Doesn't move on its own.
                break;
            case Camera::CenterOnEntity:
                moveCameraToPosition(camera, position);
                break;
            default:
                break;
        }
    }
}

void CameraSystem::moveCameraToPosition(Camera& camera, Position& position)
{
    // Move the camera.
    camera.position.x = position.x;
    camera.position.y = position.y;
}

} // namespace Client
} // namespace AM
