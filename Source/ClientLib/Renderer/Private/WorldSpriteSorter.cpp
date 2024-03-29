#include "WorldSpriteSorter.h"
#include "TileMap.h"
#include "GraphicData.h"
#include "UserInterface.h"
#include "Camera.h"
#include "SharedConfig.h"
#include "MovementHelpers.h"
#include "Transforms.h"
#include "ClientTransforms.h"
#include "TileExtent.h"
#include "SharedConfig.h"
#include "SpriteRenderData.h"
#include "Collision.h"
#include "GraphicState.h"
#include "ClientGraphicState.h"
#include "VariantTools.h"
#include <SDL_rect.h>
#include <cmath>
#include <algorithm>

namespace AM
{
namespace Client
{
WorldSpriteSorter::WorldSpriteSorter(entt::registry& inRegistry,
                                     const TileMap& inTileMap,
                                     const GraphicData& inGraphicData,
                                     const UserInterface& inUI)
: registry(inRegistry)
, tileMap{inTileMap}
, graphicData{inGraphicData}
, ui{inUI}
, phantomSprites{}
, spriteColorMods{}
, floorSprites{}
, floorCoveringSprites{}
, sortedSprites{}
, spritesToSort{}
, animationTimer{}
, lastAnimationTimestamp{}
, currentAnimationTimestamp{}
{
}

void WorldSpriteSorter::sortSprites(const Camera& camera, double alpha)
{
    // Get our new timestamp.
    lastAnimationTimestamp = currentAnimationTimestamp;
    currentAnimationTimestamp = animationTimer.getTime();

    // Clear the old data.
    sortedSprites.clear();

    // Gather sprites relevant to this frame and calc their screen extents.
    gatherSpriteInfo(camera, alpha);

    // Sort the sprites by visual depth.
    sortSpritesByDepth();

    // Push the now-sorted sprites into the sorted vector.
    sortedSprites.insert(sortedSprites.end(),
                         std::make_move_iterator(spritesToSort.begin()),
                         std::make_move_iterator(spritesToSort.end()));
    spritesToSort.clear();
}

const std::vector<SpriteSortInfo>& WorldSpriteSorter::getSortedSprites()
{
    // Return the sorted vector of sprites.
    return sortedSprites;
}

void WorldSpriteSorter::gatherSpriteInfo(const Camera& camera, double alpha)
{
    // Save a temporary copy of the UI's current phantoms and color mods.
    phantomSprites = ui.getPhantomSprites();
    spriteColorMods = ui.getSpriteColorMods();

    // Gather all relevant tiles.
    gatherTileSpriteInfo(camera);

    // Gather all relevant entities.
    gatherEntitySpriteInfo(camera, alpha);

    // Clear our temporary vectors.
    phantomSprites.clear();
    spriteColorMods.clear();
}

void WorldSpriteSorter::gatherTileSpriteInfo(const Camera& camera)
{
    // Gather all tiles that are in view.
    TileExtent tileViewExtent{
        camera.getTileViewExtent(tileMap.getTileExtent())};
    for (int y = tileViewExtent.y; y <= tileViewExtent.yMax(); ++y) {
        for (int x = tileViewExtent.x; x <= tileViewExtent.xMax(); ++x) {
            // Push all of this tile's sprites into the appropriate vectors.
            const Tile& tile{tileMap.getTile(x, y)};
            pushFloorSprite(tile, camera, x, y);
            pushFloorCoveringSprites(tile, camera, x, y);

            // TODO: We temporarily are pushing objects before walls to fix
            //       NW gap fills rendering in front of overlapping objects.
            //       We should instead find a way to make objects and phantoms
            //       consistently render in front of walls.
            pushObjectSprites(tile, camera, x, y);
            pushWallSprites(tile, camera, x, y);
        }
    }

    // Gather all of the UI's phantom tile sprites that weren't already used.
    for (const PhantomSpriteInfo& info : phantomSprites) {
        if (info.layerType != TileLayer::Type::None) {
            GraphicRef graphic{*(info.sprite)};
            pushTileSprite(graphic, camera,
                           {info.tileX, info.tileY, info.layerType, 0, 0},
                           true);
        }
    }

    // Add all of the floors, then all of the floor coverings to the sorted
    // sprites vector so they're in the correct rendering order.
    // Note: The other sprites haven't been sorted yet, so the vector is empty.
    sortedSprites.insert(sortedSprites.end(),
                         std::make_move_iterator(floorSprites.begin()),
                         std::make_move_iterator(floorSprites.end()));
    floorSprites.clear();
    sortedSprites.insert(sortedSprites.end(),
                         std::make_move_iterator(floorCoveringSprites.begin()),
                         std::make_move_iterator(floorCoveringSprites.end()));
    floorCoveringSprites.clear();
}

void WorldSpriteSorter::gatherEntitySpriteInfo(const Camera& camera,
                                               double alpha)
{
    // Gather all entities that have a Position and GraphicState.
    auto view = registry.view<Position, GraphicState, ClientGraphicState>();
    for (entt::entity entity : view) {
        auto [position, graphicState, clientGraphicState]
            = view.get<Position, GraphicState, ClientGraphicState>(entity);

        // If this entity has a previous position, calc a lerp'd position.
        Position renderPosition{position};
        if (registry.all_of<PreviousPosition>(entity)) {
            const auto& previousPos{registry.get<PreviousPosition>(entity)};
            renderPosition = MovementHelpers::interpolatePosition(
                previousPos, position, alpha);
        }

        const Sprite& sprite{getEntitySprite(graphicState, clientGraphicState)};
        pushEntitySprite(entity, renderPosition, sprite, camera);
    }

    // Gather all of the UI's phantom entity sprites.
    for (const PhantomSpriteInfo& info : phantomSprites) {
        if (info.layerType == TileLayer::Type::None) {
            pushEntitySprite(entt::null, info.position, *(info.sprite), camera);
        }
    }
}

void WorldSpriteSorter::pushFloorSprite(const Tile& tile, const Camera& camera,
                                        int x, int y)
{
    const FloorTileLayer& floor{tile.getFloor()};
    std::optional<GraphicRef> floorGraphic{floor.getGraphic()};
    if (floorGraphic) {
        // If the UI wants this sprite replaced with a phantom, replace it.
        auto phantomSpriteInfo = std::find_if(
            phantomSprites.begin(), phantomSprites.end(),
            [&](const PhantomSpriteInfo& info) {
                return ((info.layerType == TileLayer::Type::Floor)
                        && (info.tileX == x) && (info.tileY == y));
            });
        if (phantomSpriteInfo != phantomSprites.end()) {
            floorGraphic.emplace(*(phantomSpriteInfo->sprite));
            phantomSprites.erase(phantomSpriteInfo);
        }

        pushTileSprite(
            *floorGraphic, camera,
            {x, y, TileLayer::Type::Floor, floor.graphicSet->numericID, 0},
            false);
    }
}

void WorldSpriteSorter::pushFloorCoveringSprites(const Tile& tile,
                                                 const Camera& camera, int x,
                                                 int y)
{
    const auto& floorCoverings{tile.getFloorCoverings()};
    for (const FloorCoveringTileLayer& floorCovering : floorCoverings) {
        std::optional<GraphicRef> floorCoveringGraphic{
            floorCovering.getGraphic()};
        if (floorCoveringGraphic) {
            pushTileSprite(*floorCoveringGraphic, camera,
                           {x, y, TileLayer::Type::FloorCovering,
                            floorCovering.graphicSet->numericID,
                            floorCovering.direction},
                           false);
        }
    }
}

void WorldSpriteSorter::pushWallSprites(const Tile& tile, const Camera& camera,
                                        int x, int y)
{
    const std::array<WallTileLayer, 2>& walls{tile.getWalls()};
    for (const WallTileLayer& wall : walls) {
        std::optional<GraphicRef> wallGraphic{wall.getGraphic()};
        if (wallGraphic) {
            // If the UI wants this sprite replaced with a phantom, replace it.
            auto phantomSpriteInfo = std::find_if(
                phantomSprites.begin(), phantomSprites.end(),
                [&](const PhantomSpriteInfo& info) {
                    if ((info.layerType == TileLayer::Type::Wall)
                        && (info.tileX == x) && (info.tileY == y)) {
                        // Check if we need to replace a N with a NE fill.
                        if ((wall.wallType == Wall::Type::North)
                            && (info.wallType
                                == Wall::Type::NorthEastGapFill)) {
                            return true;
                        }
                        // Check if we need to replace a NW fill with a W or N.
                        else if ((wall.wallType == Wall::Type::NorthWestGapFill)
                                 && ((info.wallType == Wall::Type::West)
                                     || (info.wallType == Wall::Type::North))) {
                            return true;
                        }
                        else if (info.wallType == wall.wallType) {
                            // Otherwise, check if the type matches.
                            return true;
                        }
                    }
                    return false;
                });
            if (phantomSpriteInfo != phantomSprites.end()) {
                wallGraphic.emplace(*(phantomSpriteInfo->sprite));
                phantomSprites.erase(phantomSpriteInfo);
            }

            pushTileSprite(*wallGraphic, camera,
                           {x, y, TileLayer::Type::Wall,
                            wall.graphicSet->numericID, wall.wallType},
                           false);
        }
    }
}

void WorldSpriteSorter::pushObjectSprites(const Tile& tile,
                                          const Camera& camera, int x, int y)
{
    const std::vector<ObjectTileLayer>& objects{tile.getObjects()};
    for (const ObjectTileLayer& object : objects) {
        std::optional<GraphicRef> objectGraphic{object.getGraphic()};
        if (objectGraphic) {
            pushTileSprite(*objectGraphic, camera,
                           {x, y, TileLayer::Type::Object,
                            object.graphicSet->numericID, object.direction},
                           false);
        }
    }
}

void WorldSpriteSorter::pushTileSprite(const GraphicRef& graphic,
                                       const Camera& camera,
                                       const TileLayerID& layerID,
                                       bool isFullPhantom)
{
    // Get the current sprite for this graphic.
    const Sprite& sprite{graphic.getSpriteAtTime(animationTimer.getTime())};

    // Get iso screen extent for this sprite.
    const SpriteRenderData& renderData{
        graphicData.getRenderData(sprite.numericID)};
    SDL_FRect screenExtent{ClientTransforms::tileToScreenExtent(
        {layerID.x, layerID.y}, renderData, camera)};

    // If this sprite isn't on screen, skip it.
    if (!isWithinScreenBounds(screenExtent, camera)) {
        return;
    }

    // If the UI wants a color mod on this sprite, use it.
    SDL_Color colorMod{getColorMod<TileLayerID>(layerID)};

    // If this sprite comes from an existing tile layer or is a phantom that
    // replaces an existing sprite, set the layer's ID. Otherwise, leave it
    // as std::monostate to show that this is a full phantom.
    WorldObjectID worldObjectID{};
    if (!isFullPhantom) {
        worldObjectID = layerID;
    }

    // If this sprite is on a wall or object layer, push it to be sorted.
    if ((layerID.type == TileLayer::Type::Wall)
        || (layerID.type == TileLayer::Type::Object)) {
        Position tilePosition{
            static_cast<float>(layerID.x * SharedConfig::TILE_WORLD_WIDTH),
            static_cast<float>(layerID.y * SharedConfig::TILE_WORLD_WIDTH), 0};
        BoundingBox worldBounds{
            Transforms::modelToWorld(sprite.modelBounds, tilePosition)};
        spritesToSort.emplace_back(&sprite, worldObjectID, worldBounds,
                                   screenExtent, colorMod);
    }
    else if (layerID.type == TileLayer::Type::Floor) {
        // Push floors into their intermediate vector.
        floorSprites.emplace_back(&sprite, worldObjectID, BoundingBox{},
                                  screenExtent, colorMod);
    }
    else if (layerID.type == TileLayer::Type::FloorCovering) {
        // Push floor coverings into their intermediate vector.
        Position tilePosition{
            static_cast<float>(layerID.x * SharedConfig::TILE_WORLD_WIDTH),
            static_cast<float>(layerID.y * SharedConfig::TILE_WORLD_WIDTH), 0};
        BoundingBox worldBounds{
            Transforms::modelToWorld(sprite.modelBounds, tilePosition)};
        floorCoveringSprites.emplace_back(&sprite, worldObjectID, worldBounds,
                                          screenExtent, colorMod);
    }
    else {
        LOG_ERROR("Invalid layer type.");
    }
}

const Sprite&
    WorldSpriteSorter::getEntitySprite(const GraphicState& graphicState,
                                       ClientGraphicState& clientGraphicState)
{
    // Get the current sprite for this graphic.
    const EntityGraphicSet& graphicSet{
        graphicData.getEntityGraphicSet(graphicState.graphicSetID)};
    const GraphicRef& graphic{
        graphicSet.graphics.at(clientGraphicState.graphicType)};
    const Sprite* sprite{nullptr};
    std::visit(VariantTools::Overload{
        [&](std::reference_wrapper<const Sprite> spriteRef) {
            sprite = &(spriteRef.get());
        },
        [&](std::reference_wrapper<const Animation> animation) {
            // Calc how far we are into this animation and get the appropriate
            // sprite.
            double animationTime{currentAnimationTimestamp
                                 - clientGraphicState.animationStartTime};
            sprite = &(animation.get().getSpriteAtTime(animationTime));

            // If this animation just began, set its start time.
            if (clientGraphicState.setStartTime) {
                clientGraphicState.animationStartTime = animationTimer.getTime();
                clientGraphicState.setStartTime = false;
            }
        }
    }, graphic);

    return *sprite;
}

void WorldSpriteSorter::pushEntitySprite(entt::entity entity,
                                         const Position& position,
                                         const Sprite& sprite,
                                         const Camera& camera)
{
    // Get the iso screen extent for the sprite.
    const SpriteRenderData& renderData{
        graphicData.getRenderData(sprite.numericID)};
    SDL_FRect screenExtent{
        ClientTransforms::entityToScreenExtent(position, renderData, camera)};

    // If the sprite is on screen, push the render info.
    if (isWithinScreenBounds(screenExtent, camera)) {
        // Get an updated bounding box for this entity.
        BoundingBox worldBox{
            Transforms::modelToWorldCentered(sprite.modelBounds, position)};

        // If the UI wants a color mod on this sprite, use it.
        SDL_Color colorMod{getColorMod<entt::entity>(entity)};

        // If this sprite doesn't come from a phantom, set the owner ID.
        WorldObjectID ownerID{std::monostate{}};
        if (entity != entt::null) {
            ownerID = entity;
        }

        // Push the entity's render info for this frame.
        spritesToSort.emplace_back(&sprite, ownerID, worldBox, screenExtent,
                                   colorMod);
    }
}

void WorldSpriteSorter::sortSpritesByDepth()
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

void WorldSpriteSorter::calcDepthDependencies()
{
    // Calculate all dependencies.
    for (std::size_t i = 0; i < spritesToSort.size(); ++i) {
        for (std::size_t j = 0; j < spritesToSort.size(); ++j) {
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

void WorldSpriteSorter::visitSprite(SpriteSortInfo& spriteInfo, int& depthValue)
{
    if (!(spriteInfo.visited)) {
        spriteInfo.visited = true;

        for (std::size_t i = 0; i < spriteInfo.spritesBehind.size(); ++i) {
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

// TEMP: Remove these when we upgrade SDL
SDL_bool SDL_RectEmptyFloat(const SDL_FRect* r)
{
    return ((!r) || (r->w <= 0.0f) || (r->h <= 0.0f)) ? SDL_TRUE : SDL_FALSE;
}

SDL_bool SDL_HasRectIntersectionFloat(const SDL_FRect* A, const SDL_FRect* B)
{
    float Amin, Amax, Bmin, Bmax;

    if (!A) {
        SDL_InvalidParamError("A");
        return SDL_FALSE;
    }
    else if (!B) {
        SDL_InvalidParamError("B");
        return SDL_FALSE;
    }
    else if (SDL_RectEmptyFloat(A) || SDL_RectEmptyFloat(B)) {
        return SDL_FALSE; /* Special cases for empty rects */
    }

    /* Horizontal intersection */
    Amin = A->x;
    Amax = Amin + A->w;
    Bmin = B->x;
    Bmax = Bmin + B->w;
    if (Bmin > Amin) {
        Amin = Bmin;
    }
    if (Bmax < Amax) {
        Amax = Bmax;
    }
    if (Amax <= Amin) {
        return SDL_FALSE;
    }
    /* Vertical intersection */
    Amin = A->y;
    Amax = Amin + A->h;
    Bmin = B->y;
    Bmax = Bmin + B->h;
    if (Bmin > Amin) {
        Amin = Bmin;
    }
    if (Bmax < Amax) {
        Amax = Bmax;
    }
    if (Amax <= Amin) {
        return SDL_FALSE;
    }
    return SDL_TRUE;
}
// END TEMP: Remove these when we upgrade SDL

bool WorldSpriteSorter::isWithinScreenBounds(const SDL_FRect& extent,
                                             const Camera& camera)
{
    // The extent is in final screen coordinates, so we only need to check if
    // it's within the rect formed by (0, 0) and (camera.width, camera.height).
    SDL_FRect cameraExtent{0, 0, camera.extent.w, camera.extent.h};
    return (SDL_HasRectIntersectionFloat(&extent, &cameraExtent) == SDL_TRUE);
}

} // End namespace Client
} // End namespace AM
