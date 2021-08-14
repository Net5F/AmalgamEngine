#include "WorldSpritePreparer.h"
#include "TileMap.h"
#include "Camera.h"
#include "SharedConfig.h"
#include "MovementHelpers.h"
#include "Transforms.h"
#include "ClientTransforms.h"

#include <SDL2/SDL_rect.h>

#include <algorithm>

namespace AM
{
namespace Client
{
WorldSpritePreparer::WorldSpritePreparer(entt::registry& inRegistry,
                                         const TileMap& inTileMap)
: registry(inRegistry)
, tileMap{inTileMap}
, sortedSprites{}
, spritesToSort{}
{
}

std::vector<SpriteRenderInfo>&
    WorldSpritePreparer::prepareSprites(const Camera& camera, double alpha)
{
    // Clear the old data.
    sortedSprites.clear();
    spritesToSort.clear();

    // Gather sprites relevant to this frame and calc their screen extents.
    gatherSpriteInfo(camera, alpha);

    // Sort the sprites by visual depth.
    sortSpritesByDepth();

    // Return the sorted vector of sprites.
    return sortedSprites;
}

void WorldSpritePreparer::gatherSpriteInfo(const Camera& camera, double alpha)
{
    // Gather tiles.
    for (int y = 0; y < static_cast<int>(SharedConfig::WORLD_HEIGHT); ++y) {
        for (int x = 0; x < static_cast<int>(SharedConfig::WORLD_WIDTH);
                 ++x) {
                // Figure out which tile we're looking at.
                const Tile& tile = tileMap.get(x, y);

                // Push all of this tile's sprites into the appropriate vector.
                for (const Tile::SpriteLayer& layer : tile.spriteLayers) {
                    // Get iso screen extent for this sprite.
                    SDL_Rect screenExtent
                        = ClientTransforms::tileToScreenExtent({x, y}, *(layer.sprite), camera);

                    // If this sprite isn't on screen, skip it.
                    if (isWithinScreenBounds(screenExtent, camera)) {
                        continue;
                    }

                    // If this sprite has a bounding box, push it to be sorted.
                    if (layer.sprite->hasBoundingBox) {
                        spritesToSort.emplace_back(layer.sprite, layer.fixedBounds, screenExtent);
                    }
                    else {
                        // No bounding box, push it straight into the sorted
                        // sprites vector.
                        sortedSprites.emplace_back(layer.sprite, BoundingBox{}, screenExtent);
                    }
                }
        }
    }

    // Gather all relevant dynamic sprites.
    auto group = registry.group<Sprite>(entt::get<Position, PreviousPosition>);
    for (entt::entity entity : group) {
        auto [sprite, position, previousPos]
            = group.get<Sprite, Position, PreviousPosition>(entity);

        // Get the entity's lerp'd world position.
        Position lerp = MovementHelpers::interpolatePosition(previousPos,
                                                             position, alpha);

        // Get the iso screen extent for the lerped sprite.
        SDL_Rect screenExtent
            = ClientTransforms::worldToScreenExtent(lerp, sprite, camera);

        // If the sprite is on screen, push the render info.
        if (isWithinScreenBounds(screenExtent, camera)) {
            // Get an updated bounding box for this entity.
            BoundingBox movedBox = MovementHelpers::moveBoundingBox(position, sprite.modelBounds);

            // Push the entity's render info for this frame.
            spritesToSort.emplace_back(&sprite, movedBox, screenExtent);
        }
    }
}

void WorldSpritePreparer::sortSpritesByDepth()
{
    // Calculate dependencies (who is behind who).
    calcDepthDependencies();

    // Calculate depth values.
    int depthValue = 0;
    for (SpriteRenderInfo& spriteInfo : spritesToSort) {
        visitSprite(spriteInfo, depthValue);
    }

    // Sort sprites by depth.
    std::sort(
        spritesToSort.begin(), spritesToSort.end(),
        [](const SpriteRenderInfo& lhs, const SpriteRenderInfo& rhs) -> bool {
            return lhs.depthValue < rhs.depthValue;
        });
}

void WorldSpritePreparer::calcDepthDependencies()
{
    // Calculate all dependencies.
    for (unsigned int i = 0; i < spritesToSort.size(); ++i) {
        for (unsigned int j = 0; j < spritesToSort.size();
             ++j) {
            if (i != j) {
                SpriteRenderInfo& spriteA = spritesToSort[i];
                SpriteRenderInfo& spriteB = spritesToSort[j];

                if ((spriteB.worldBounds.minX < spriteA.worldBounds.maxX)
                    && (spriteB.worldBounds.minY < spriteA.worldBounds.maxY)
                    && (spriteB.worldBounds.minZ < spriteA.worldBounds.maxZ)) {
                    // B is behind A, push it into A.spritesBehind.
                    spriteA.spritesBehind.push_back(&spriteB);
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
