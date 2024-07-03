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

        // Update the camera's view bounds.
        camera.viewBounds.center.x = camera.target.x;
        camera.viewBounds.center.y = camera.target.y;
        camera.viewBounds.halfExtents.x = SharedConfig::VIEW_RADIUS;
        camera.viewBounds.halfExtents.y = SharedConfig::VIEW_RADIUS;
        // Note: The camera can always see down to Z == 0.
        float zHalfLength{(camera.target.z + SharedConfig::VIEW_RADIUS) / 2.f};
        camera.viewBounds.center.z = zHalfLength;
        camera.viewBounds.halfExtents.z = zHalfLength;
        // Note: We purposely don't clip to the tile map's bounds, because it's 
        //       reasonable to view things outside of the map.
    }
}

} // namespace Client
} // namespace AM
