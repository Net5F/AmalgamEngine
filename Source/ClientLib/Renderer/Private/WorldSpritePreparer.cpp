#include "WorldSpritePreparer.h"
#include "TileMap.h"
#include "Camera.h"
#include "SharedConfig.h"
#include "MovementHelpers.h"
#include "Transforms.h"
#include "ClientTransforms.h"
#include "TileExtent.h"
#include "SharedConfig.h"
#include "SpriteData.h"
#include "SpriteRenderData.h"
#include "Ignore.h"
#include <SDL_rect.h>
#include <cmath>
#include <algorithm>

namespace AM
{
namespace Client
{
WorldSpritePreparer::WorldSpritePreparer(entt::registry& inRegistry,
                                         const TileMap& inTileMap,
                                         const SpriteData& inSpriteData)
: registry(inRegistry)
, tileMap{inTileMap}
, spriteData{inSpriteData}
, sortedSprites{}
, spritesToSort{}
{
}

std::vector<SpriteSortInfo>&
    WorldSpritePreparer::prepareSprites(const Camera& camera, double alpha)
{
    // Clear the old data.
    sortedSprites.clear();
    spritesToSort.clear();

    // Gather sprites relevant to this frame and calc their screen extents.
    gatherSpriteInfo(camera, alpha);

    // Sort the sprites by visual depth.
    sortSpritesByDepth();

    // Push the now-sorted sprites into the sorted vector.
    for (SpriteSortInfo& sprite : spritesToSort) {
        sortedSprites.push_back(sprite);
    }

    // Return the sorted vector of sprites.
    return sortedSprites;
}

void WorldSpritePreparer::gatherSpriteInfo(const Camera& camera, double alpha)
{
    // Find the world position that the camera is centered on.
    ScreenPoint centerPoint{(camera.extent.width / 2),
                            (camera.extent.height / 2)};
    Position centerPosition{Transforms::screenToWorld(centerPoint, camera)};

    // Issues with float precision can cause flickering tiles. Round to the
    // nearest whole number to avoid this.
    centerPosition.x = std::round(centerPosition.x);
    centerPosition.y = std::round(centerPosition.y);

    // Find the lowest x/y tile indices that the player can see.
    TileExtent tileViewExtent{};
    tileViewExtent.x = static_cast<int>(
        std::floor((centerPosition.x - SharedConfig::VIEW_RADIUS)
                   / static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)));
    tileViewExtent.y = static_cast<int>(
        std::floor((centerPosition.y - SharedConfig::VIEW_RADIUS)
                   / static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)));

    // Calc how far the player's view extends.
    // Note: We add 1 to the view radius to keep all sides even, since the
    //       player occupies a tile.
    tileViewExtent.xLength
        = static_cast<int>(std::ceil(((SharedConfig::VIEW_RADIUS * 2) + 1)
                                     / SharedConfig::TILE_WORLD_WIDTH));
    tileViewExtent.yLength = tileViewExtent.xLength;

    // Clip the view to the world bounds.
    tileViewExtent.intersectWith(tileMap.getTileExtent());

    // Gather all tiles that are in view.
    for (int y = tileViewExtent.y; y <= tileViewExtent.yMax(); ++y) {
        for (int x = tileViewExtent.x; x <= tileViewExtent.xMax(); ++x) {
            // Get the tile data that we want to process.
            const Tile& tile{tileMap.getTile(x, y)};

            // Push all of this tile's sprites into the appropriate vector.
            for (const Tile::SpriteLayer& layer : tile.spriteLayers) {
                // If the layer is empty, skip it.
                if (layer.sprite.numericID == EMPTY_SPRITE_ID) {
                    continue;
                }

                // Get iso screen extent for this sprite.
                const SpriteRenderData& renderData{
                    spriteData.getRenderData(layer.sprite.numericID)};
                SDL_Rect screenExtent{ClientTransforms::tileToScreenExtent(
                    {x, y}, renderData, camera)};

                // If this sprite isn't on screen, skip it.
                if (!isWithinScreenBounds(screenExtent, camera)) {
                    continue;
                }

                // If this sprite has a bounding box, push it to be sorted.
                if (layer.sprite.hasBoundingBox) {
                    spritesToSort.emplace_back(&(layer.sprite),
                                               layer.worldBounds, screenExtent);
                }
                else {
                    // No bounding box, push it straight into the sorted
                    // sprites vector.
                    sortedSprites.emplace_back(&(layer.sprite), BoundingBox{},
                                               screenExtent);
                }
            }
        }
    }

    // Gather all relevant dynamic sprites.
    auto group = registry.group<Position, PreviousPosition, Sprite>();
    for (entt::entity entity : group) {
        auto [sprite, position, previousPos]
            = group.get<Sprite, Position, PreviousPosition>(entity);

        // Get the entity's lerp'd world position.
        Position lerp{
            MovementHelpers::interpolatePosition(previousPos, position, alpha)};

        // Get the iso screen extent for the lerped sprite.
        const SpriteRenderData& renderData{
            spriteData.getRenderData(sprite.numericID)};
        SDL_Rect screenExtent{
            ClientTransforms::entityToScreenExtent(lerp, renderData, camera)};

        // If the sprite is on screen, push the render info.
        if (isWithinScreenBounds(screenExtent, camera)) {
            // Get an updated bounding box for this entity.
            BoundingBox worldBox{
                Transforms::modelToWorldCentered(sprite.modelBounds, lerp)};

            // Push the entity's render info for this frame.
            spritesToSort.emplace_back(&sprite, worldBox, screenExtent);
        }
    }
}

void WorldSpritePreparer::sortSpritesByDepth()
{
    // Calculate dependencies (who is behind who).
    calcDepthDependencies();

    // Calculate depth values.
    int depthValue{0};
    for (SpriteSortInfo& spriteInfo : spritesToSort) {
        visitSprite(spriteInfo, depthValue);
    }

    // Sort sprites by depth.
    std::sort(spritesToSort.begin(), spritesToSort.end(),
              [](const SpriteSortInfo& lhs, const SpriteSortInfo& rhs) -> bool {
                  return lhs.depthValue < rhs.depthValue;
              });
}

void WorldSpritePreparer::calcDepthDependencies()
{
    // Calculate all dependencies.
    for (unsigned int i = 0; i < spritesToSort.size(); ++i) {
        for (unsigned int j = 0; j < spritesToSort.size(); ++j) {
            if (i != j) {
                SpriteSortInfo& spriteA{spritesToSort[i]};
                SpriteSortInfo& spriteB{spritesToSort[j]};

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

void WorldSpritePreparer::visitSprite(SpriteSortInfo& spriteInfo,
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
    SDL_Rect cameraExtent{0, 0, static_cast<int>(camera.extent.width),
                          static_cast<int>(camera.extent.height)};
    return (SDL_HasIntersection(&extent, &cameraExtent) == SDL_TRUE);
}

} // End namespace Client
} // End namespace AM
