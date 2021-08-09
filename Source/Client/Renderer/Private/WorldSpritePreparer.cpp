#include "WorldSpritePreparer.h"
#include "Camera.h"
#include "SharedConfig.h"
#include "MovementHelpers.h"
#include "ClientMovementHelpers.h"
#include "Transforms.h"
#include "ClientTransforms.h"

#include <SDL2/SDL_rect.h>

#include <algorithm>

namespace AM
{
namespace Client
{
WorldSpritePreparer::WorldSpritePreparer(entt::registry& inRegistry,
                                         std::vector<TileLayer>& inMapLayers)
: registry(inRegistry)
, mapLayers(inMapLayers)
, sprites{}
, heightfulSpriteStartIndex{0}
{
}

std::vector<SpriteRenderInfo>&
    WorldSpritePreparer::prepareSprites(const Camera& camera, double alpha)
{
    // Clear the old data.
    sprites.clear();
    heightfulSpriteStartIndex = 0;

    // Update the world bounds of all dynamic sprites.
    updateSpriteWorldBounds(alpha);

    // Gather sprites relevant to this frame and calc their screen extents.
    gatherSpriteInfo(camera, alpha);

    // Sort the sprites by visual depth.
    sortSpritesByDepth();

    // Return the sorted vector of sprites.
    return sprites;
}

// TODO: Combine the update and gather steps. Update is more of a
//       calcLerpedSpriteWorldBounds(), we can re-use the calc'd lerp to do both
//       the update and the screen conversion to check isWithinScreenBounds().
//
//       Move worldBounds out of the sprite, since it's render-specific. Save it
//       in SpriteRenderInfo instead.
//
//       width/height are set on construction, but never modified. There also
//       isn't really a place to set them. Maybe just remove them and use
//       textureExtent.
//
//       Once sprite has the dynamic stuff pulled out, we can make everything
//       const and put the rest of the fields in.
//
//       Still a question: do we need separate structs for client/server?
//           Server only really cares about IDs, hasBoundingBox, and modelBounds
//               Should have separate SpriteData classes, using separate
//               structs.

void WorldSpritePreparer::updateSpriteWorldBounds(double alpha)
{
    // Update all sprites that are on dynamic (moving) entities.
    auto group = registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    for (entt::entity entity : group) {
        auto [sprite, position, previousPos]
            = group.get<Sprite, Position, PreviousPosition>(entity);

        // Get the lerp'd world position.
        Position lerp = MovementHelpers::interpolatePosition(previousPos,
                                                             position, alpha);

        // Update the sprite's world bounds.
        ClientMovementHelpers::moveSpriteWorldBounds(lerp, sprite);
    }
}

void WorldSpritePreparer::gatherSpriteInfo(const Camera& camera, double alpha)
{
    // Gather tiles.
    for (unsigned int i = 0; i < mapLayers.size(); ++i) {
        for (int y = 0; y < static_cast<int>(SharedConfig::WORLD_HEIGHT); ++y) {
            for (int x = 0; x < static_cast<int>(SharedConfig::WORLD_WIDTH);
                 ++x) {
                unsigned int linearizedIndex
                    = y * SharedConfig::WORLD_WIDTH + x;
                Sprite& sprite = mapLayers[i][linearizedIndex];

                // If there's nothing in this tile, skip it.
                if (sprite.texture == nullptr) {
                    continue;
                }
                else {
                    // Get iso screen extent for this tile.
                    Sprite& sprite{mapLayers[i][linearizedIndex]};
                    SDL_Rect screenExtent
                        = ClientTransforms::tileToScreenExtent({x, y}, sprite,
                                                               camera);

                    // If the sprite is on screen, push the sprite info.
                    if (isWithinScreenBounds(screenExtent, camera)) {
                        sprites.emplace_back(&sprite, screenExtent);

                        // If the tile has no height, increment the index.
                        // TODO: Get this number from the map object.
                        if (i < 2) {
                            heightfulSpriteStartIndex++;
                        }
                    }
                }
            }
        }
    }

    // Gather all relevant dynamic sprites.
    auto group = registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    for (entt::entity entity : group) {
        auto [sprite, position, previousPos]
            = group.get<Sprite, Position, PreviousPosition>(entity);

        // Get the lerp'd world position.
        Position lerp = MovementHelpers::interpolatePosition(previousPos,
                                                             position, alpha);

        // Get the iso screen extent for the lerped sprite.
        SDL_Rect screenExtent
            = ClientTransforms::worldToScreenExtent(lerp, sprite, camera);

        // If the sprite is on screen, push the sprite info.
        if (isWithinScreenBounds(screenExtent, camera)) {
            sprites.emplace_back(&sprite, screenExtent);
        }
    }
}

void WorldSpritePreparer::sortSpritesByDepth()
{
    // Calculate dependencies (who is behind who).
    calcDepthDependencies();

    // Calculate depth values for heightful sprites.
    int depthValue = 0;
    for (unsigned int i = heightfulSpriteStartIndex; i < sprites.size(); ++i) {
        visitSprite(sprites[i], depthValue);
    }

    // Sort heightful sprites by depth.
    std::sort(
        (sprites.begin() + heightfulSpriteStartIndex), sprites.end(),
        [](const SpriteRenderInfo& lhs, const SpriteRenderInfo& rhs) -> bool {
            return lhs.depthValue < rhs.depthValue;
        });
}

void WorldSpritePreparer::calcDepthDependencies()
{
    // Calculate all dependencies.
    for (unsigned int i = heightfulSpriteStartIndex; i < sprites.size(); ++i) {
        for (unsigned int j = heightfulSpriteStartIndex; j < sprites.size();
             ++j) {
            if (i != j) {
                Sprite& spriteA = *(sprites[i].sprite);
                Sprite& spriteB = *(sprites[j].sprite);

                if ((spriteB.worldBounds.minX < spriteA.worldBounds.maxX)
                    && (spriteB.worldBounds.minY < spriteA.worldBounds.maxY)
                    && (spriteB.worldBounds.minZ < spriteA.worldBounds.maxZ)) {
                    // B is behind A, push it into A.spritesBehind.
                    sprites[i].spritesBehind.push_back(&(sprites[j]));
                }
            }
        }
    }
}

void WorldSpritePreparer::visitSprite(SpriteRenderInfo& spriteInfo,
                                      int& depthValue)
{
    if (!(spriteInfo.visited)) {
        spriteInfo.visited = true;

        for (unsigned int i = 0; i < spriteInfo.spritesBehind.size(); ++i) {
            if (spriteInfo.spritesBehind[i] == nullptr) {
                break;
            }
            else {
                visitSprite(*(spriteInfo.spritesBehind[i]), depthValue);
                spriteInfo.spritesBehind[i] = nullptr;
            }
        }

        spriteInfo.depthValue = depthValue;
        depthValue++;
    }
}

bool WorldSpritePreparer::isWithinScreenBounds(const SDL_Rect& extent,
                                               const Camera& camera)
{
    // The extent is in final screen coordinates, so we only need to check if
    // it's within the rect formed by (0, 0) and (camera.width, camera.height).
    bool pastLeftBound = ((extent.x + extent.w) < 0);
    bool pastRightBound = (camera.extent.width < extent.x);
    bool pastTopBound = ((extent.y + extent.h) < 0);
    bool pastBottomBound = (camera.extent.height < extent.y);

    // If the extent is outside of any camera bound, return false.
    if (pastLeftBound || pastRightBound || pastTopBound || pastBottomBound) {
        return false;
    }
    else {
        // Extent is within the camera bounds, return true.
        return true;
    }
}

} // End namespace Client
} // End namespace AM
