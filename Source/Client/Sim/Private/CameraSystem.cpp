#include "CameraSystem.h"
#include "Sim.h"
#include "World.h"
#include "Camera.h"
#include "Position.h"
#include "Ignore.h"
#include "Log.h"

namespace AM
{
namespace Client
{
CameraSystem::CameraSystem(Sim& inSim, World& inWorld)
: sim(inSim)
, world(inWorld)
{
    // Init the groups that we'll be using.
    auto cameraGroup = world.registry.group<Camera>(entt::get<Position>);
    ignore(cameraGroup);
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
                centerCameraOnEntity(camera, position, entity);
                break;
            default:
                break;
        }
    }
}

void CameraSystem::centerCameraOnEntity(Camera& camera, Position& position, entt::entity entity)
{
    // If the entity has a sprite, include its size in the centering.
    unsigned int spriteWidth = 0;
    unsigned int spriteHeight = 0;
    if (world.registry.has<Sprite>(entity)) {
        Sprite& sprite = world.registry.get<Sprite>(entity);
        spriteWidth = sprite.width;
        spriteHeight = sprite.height;
    }

    // Center the camera, accounting for the sprite if applicable.
    camera.position.x = position.x - ((camera.width - spriteWidth) / 2);
    camera.position.y = position.y - ((camera.height - spriteHeight) / 2);
}

} // namespace Client
} // namespace AM
