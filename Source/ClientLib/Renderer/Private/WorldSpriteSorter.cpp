#include "WorldSpriteSorter.h"
#include "TileMap.h"
#include "GraphicData.h"
#include "UserInterface.h"
#include "Camera.h"
#include "SharedConfig.h"
#include "MovementHelpers.h"
#include "Transforms.h"
#include "ClientTransforms.h"
#include "TileLayer.h"
#include "TileLayerID.h"
#include "TileExtent.h"
#include "SharedConfig.h"
#include "SpriteRenderData.h"
#include "Collision.h"
#include "GraphicState.h"
#include "ClientGraphicState.h"
#include "Floor.h"
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
    for (int z{tileViewExtent.z}; z <= tileViewExtent.zMax(); ++z) {
        for (int y{tileViewExtent.y}; y <= tileViewExtent.yMax(); ++y) {
            for (int x{tileViewExtent.x}; x <= tileViewExtent.xMax(); ++x) {
                TilePosition tilePosition{x, y, z};

                // Push all of this tile's sprites into the appropriate vectors.
                // Note: tile will be nullptr if the chunk is empty.
                if (const Tile* tile{tileMap.cgetTile(tilePosition)}) {
                    pushTerrainSprites(*tile, camera, tilePosition);
                    pushFloorSprite(*tile, camera, tilePosition);
                    pushWallSprites(*tile, camera, tilePosition);
                    pushObjectSprites(*tile, camera, tilePosition);
                }
            }
        }
    }

    // Gather all of the UI's phantom tile sprites that weren't already used.
    for (const PhantomSpriteInfo& info : phantomSprites) {
        if (info.layerType != TileLayer::Type::None) {
            GraphicRef graphic{getPhantomGraphic(info)};

            // If it's a wall phantom, make it match the terrain height.
            TileOffset tileOffset{info.tileOffset};
            if (info.layerType == TileLayer::Type::Wall) {
                tileOffset.z = getTerrainHeight(info.tilePosition);
            }

            pushTileSprite(graphic, camera,
                           {info.tilePosition, tileOffset, info.layerType,
                            info.graphicSet->numericID, info.graphicValue},
                           true);
        }
    }
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
            GraphicRef graphic{getPhantomGraphic(info)};
            pushEntitySprite(entt::null, info.position,
                             graphic.getFirstSprite(), camera);
        }
    }
}

void WorldSpriteSorter::pushTerrainSprites(
    const Tile& tile, const Camera& camera, const TilePosition& tilePosition)
{
    // Note: Each tile can only have 1 terrain, but we get a span just to 
    //       follow the pattern of the other types.
    std::span<const TileLayer> terrains{
        tile.getLayers(TileLayer::Type::Terrain)};
    for (const TileLayer& terrain : terrains) {
        GraphicRef graphic{terrain.getGraphic()};
        if (graphic.getGraphicID() != NULL_GRAPHIC_ID) {
            Uint8 graphicValue{terrain.graphicValue};

            // If the UI wants this sprite replaced with a phantom, replace it.
            auto phantomSpriteInfo = std::find_if(
                phantomSprites.begin(), phantomSprites.end(),
                [&](const PhantomSpriteInfo& info) {
                    return ((info.layerType == TileLayer::Type::Terrain)
                            && (info.tilePosition == tilePosition));
                });
            if (phantomSpriteInfo != phantomSprites.end()) {
                graphic = getPhantomGraphic(*phantomSpriteInfo);
                graphicValue = phantomSpriteInfo->graphicValue;
                phantomSprites.erase(phantomSpriteInfo);
            }

            pushTileSprite(graphic, camera,
                           {tilePosition, TileOffset{},
                            TileLayer::Type::Terrain,
                            terrain.graphicSet.get().numericID, graphicValue},
                           false);
        }
    }
}

void WorldSpriteSorter::pushFloorSprite(const Tile& tile, const Camera& camera,
                                        const TilePosition& tilePosition)
{
    std::span<const TileLayer> floors{
        tile.getLayers(TileLayer::Type::Floor)};
    for (const TileLayer& floor : floors) {
        GraphicRef graphic{floor.getGraphic()};
        if (graphic.getGraphicID() != NULL_GRAPHIC_ID) {
            pushTileSprite(
                graphic, camera,
                {tilePosition, floor.tileOffset, TileLayer::Type::Floor,
                 floor.graphicSet.get().numericID, floor.graphicValue},
                false);
        }
    }
}

void WorldSpriteSorter::pushWallSprites(const Tile& tile, const Camera& camera,
                                        const TilePosition& tilePosition)
{
    // Make this wall match the tile's terrain height.
    // Note: Walls don't normally use offsets, but it's convenient to do it 
    //       this way.
    TileOffset tileOffset{0, 0, getTerrainHeight(tilePosition)};

    std::span<const TileLayer> walls{tile.getLayers(TileLayer::Type::Wall)};
    for (const TileLayer& wall : walls) {
        GraphicRef graphic{wall.getGraphic()};
        if (graphic.getGraphicID() != NULL_GRAPHIC_ID) {
            // If the UI wants this sprite replaced with a phantom, replace it.
            auto phantomSpriteInfo = std::find_if(
                phantomSprites.begin(), phantomSprites.end(),
                [&](const PhantomSpriteInfo& info) {
                    if ((info.layerType == TileLayer::Type::Wall)
                        && (info.tilePosition == tilePosition)) {
                        // Check if we need to replace a N with a NE fill.
                        if ((wall.graphicValue == Wall::Type::North)
                            && (info.wallType
                                == Wall::Type::NorthEastGapFill)) {
                            return true;
                        }
                        // Check if we need to replace a NW fill with a W or N.
                        else if ((wall.graphicValue
                                  == Wall::Type::NorthWestGapFill)
                                 && ((info.wallType == Wall::Type::West)
                                     || (info.wallType == Wall::Type::North))) {
                            return true;
                        }
                        else if (info.wallType == wall.graphicValue) {
                            // Otherwise, check if the type matches.
                            return true;
                        }
                    }
                    return false;
                });
            if (phantomSpriteInfo != phantomSprites.end()) {
                graphic = getPhantomGraphic(*phantomSpriteInfo);
                phantomSprites.erase(phantomSpriteInfo);
            }

            pushTileSprite(graphic, camera,
                           {tilePosition, tileOffset, TileLayer::Type::Wall,
                            wall.graphicSet.get().numericID, wall.graphicValue},
                           false);
        }
    }
}

void WorldSpriteSorter::pushObjectSprites(const Tile& tile,
                                          const Camera& camera,
                                          const TilePosition& tilePosition)
{
    std::span<const TileLayer> objects{tile.getLayers(TileLayer::Type::Object)};
    for (const TileLayer& object : objects) {
        GraphicRef graphic{object.getGraphic()};
        if (graphic.getGraphicID() != NULL_GRAPHIC_ID) {
            pushTileSprite(
                graphic, camera,
                {tilePosition, object.tileOffset, TileLayer::Type::Object,
                 object.graphicSet.get().numericID, object.graphicValue},
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

    // Get the iso screen extent for this sprite.
    const SpriteRenderData& renderData{
        graphicData.getSpriteRenderData(sprite.numericID)};
    SDL_FRect screenExtent{ClientTransforms::tileToScreenExtent(
        layerID.tilePosition, layerID.tileOffset, renderData, camera)};

    // If this is a Terrain layer, offset it based on its starting height.
    // Note: We only need to do this visually, Terrain::calcWorldBounds adds 
    //       start height to the bounds appropriately below.
    if (layerID.type == TileLayer::Type::Terrain) {
        Terrain::Height startHeight{
            Terrain::getStartHeight(layerID.graphicValue)};
        screenExtent.y -= Transforms::worldZToScreenY(
            Terrain::getHeightWorldValue(startHeight), camera.zoomFactor);
    }

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

    // Calc the sprite's world bounds.
    BoundingBox worldBounds{};
    if (layerID.type == TileLayer::Type::Terrain) {
        // For terrain, we ignore the sprite's modelBounds and instead 
        // generate a bounding volume based on the terrain type.
        worldBounds = Terrain::calcWorldBounds(layerID.tilePosition,
                                               layerID.graphicValue);
    }
    else if (layerID.type == TileLayer::Type::Floor) {
        // For floors, we ignore the sprite's modelBounds and instead 
        // generate a flat, tile-sized bounding volume.
        worldBounds = Floor::calcWorldBounds(layerID.tilePosition);
    }
    else {
        worldBounds = Transforms::modelToWorldTile(sprite.modelBounds,
                                                   layerID.tilePosition);
    }
    worldBounds.min.x += layerID.tileOffset.x;
    worldBounds.min.y += layerID.tileOffset.y;
    worldBounds.min.z += layerID.tileOffset.z;
    worldBounds.max.x += layerID.tileOffset.x;
    worldBounds.max.y += layerID.tileOffset.y;
    worldBounds.max.z += layerID.tileOffset.z;

    // Push the sprite to be sorted.
    spritesToSort.emplace_back(&sprite, worldObjectID, worldBounds,
                               screenExtent, colorMod);
}

const Sprite&
    WorldSpriteSorter::getEntitySprite(const GraphicState& graphicState,
                                       ClientGraphicState& clientGraphicState)
{
    // Get the current sprite for this graphic.
    const EntityGraphicSet& graphicSet{
        graphicData.getEntityGraphicSet(graphicState.graphicSetID)};
    AM_ASSERT(graphicSet.graphics.contains(clientGraphicState.graphicType),
              "Tried to get entity sprite that doesn't exist in set. Set: %s, "
              "Graphic type: %u",
              graphicSet.displayName.c_str(), clientGraphicState.graphicType);
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
        graphicData.getSpriteRenderData(sprite.numericID)};
    const GraphicState& graphicState{registry.get<GraphicState>(entity)};
    const EntityGraphicSet& graphicSet{
        graphicData.getEntityGraphicSet(graphicState.graphicSetID)};
    const ClientGraphicState& clientGraphicState{
        registry.get<ClientGraphicState>(entity)};

    SDL_FRect screenExtent{ClientTransforms::entityToScreenExtent(
        position, graphicSet.getCollisionModelBounds().getBottomCenterPoint(),
        graphicData.getRenderAlignmentOffset(graphicState.graphicSetID,
                                             clientGraphicState.graphicType),
        renderData, camera)};

    // If the sprite is on screen, push the render info.
    if (isWithinScreenBounds(screenExtent, camera)) {
        // Get a box for this entity, to use for sorting.
        // Note: We sort using the standard collision model bounds (from 
        //       IdleSouth). We could instead use the current sprite, but
        //       that would open us up to potentially sorting boxes that are
        //       clipping inside of eachother.
        BoundingBox worldBounds{Transforms::modelToWorldEntity(
            graphicSet.getCollisionModelBounds(), position)};

        // If the UI wants a color mod on this sprite, use it.
        SDL_Color colorMod{getColorMod<entt::entity>(entity)};

        // If this sprite doesn't come from a phantom, set the owner ID.
        WorldObjectID ownerID{std::monostate{}};
        if (entity != entt::null) {
            ownerID = entity;
        }

        // Push the entity's render info for this frame.
        spritesToSort.emplace_back(&sprite, ownerID, worldBounds, screenExtent,
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
    for (std::size_t i{0}; i < spritesToSort.size(); ++i) {
        for (std::size_t j{0}; j < spritesToSort.size(); ++j) {
            if (i != j) {
                SpriteSortInfo& spriteA{spritesToSort[i]};
                SpriteSortInfo& spriteB{spritesToSort[j]};

                if ((spriteB.worldBounds.min.x < spriteA.worldBounds.max.x)
                    && (spriteB.worldBounds.min.y < spriteA.worldBounds.max.y)
                    && (spriteB.worldBounds.min.z
                        < spriteA.worldBounds.max.z)) {
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

        for (std::size_t i{0}; i < spriteInfo.spritesBehind.size(); ++i) {
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
    SDL_FRect cameraExtent{0, 0, camera.screenExtent.w, camera.screenExtent.h};
    return (SDL_HasRectIntersectionFloat(&extent, &cameraExtent) == SDL_TRUE);
}

GraphicRef WorldSpriteSorter::getPhantomGraphic(
    const PhantomSpriteInfo& phantomSpriteInfo)
{
    // If the set is null for whatever reason, return the null sprite.
    if (!(phantomSpriteInfo.graphicSet)) {
        return GraphicRef{graphicData.getSprite(NULL_SPRITE_ID)};
    }

    // If layerType != None, this is a tile phantom.
    if (phantomSpriteInfo.layerType != TileLayer::Type::None) {
        return TileLayer::getGraphic(phantomSpriteInfo.layerType,
                                     *(phantomSpriteInfo.graphicSet),
                                     phantomSpriteInfo.graphicValue);
    }
    else {
        // Entity phantom.
        EntityGraphicType type{
            static_cast<EntityGraphicType>(phantomSpriteInfo.graphicValue)};
        const EntityGraphicSet& graphicSet{static_cast<const EntityGraphicSet&>(
            *(phantomSpriteInfo.graphicSet))};
        auto graphicIt{graphicSet.graphics.find(type)};
        if (graphicIt != graphicSet.graphics.end()) {
            return graphicIt->second;
        }
    }

    return GraphicRef{graphicData.getSprite(NULL_SPRITE_ID)};
}

Uint8 WorldSpriteSorter::getTerrainHeight(const TilePosition& tilePosition)
{
    if (const Tile* tile{tileMap.getTile(tilePosition)}) {
        if (auto* terrain{tile->findLayer(TileLayer::Type::Terrain)}) {
            Terrain::Height height{
                Terrain::getTotalHeight(terrain->graphicValue)};
            return static_cast<Uint8>(Terrain::getHeightWorldValue(height));
        }
    }

    return 0;
}

} // End namespace Client
} // End namespace AM
