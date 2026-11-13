#include "catch2/catch_all.hpp"
#include "NavTestMap.h"
#include "Pathfinder.h"
#include "EntityMover.h"
#include "EntityLocator.h"
#include "TileMapBase.h"
#include "GraphicDataBase.h"
#include "Input.h"
#include "Position.h"
#include "PreviousPosition.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "Rotation.h"
#include "Collision.h"
#include "CollisionBitSets.h"
#include "Transforms.h"
#include "entt/entity/registry.hpp"
#include "nlohmann/json.hpp"

using namespace AM;

namespace
{
/**
 * A tile map that only provides an extent. EntityMover only uses the tile map
 * for its bounds, and the tiles themselves are added straight to the
 * collision locator by NavTestMap.
 */
struct ExtentTileMap : public TileMapBase {
    ExtentTileMap(const GraphicDataBase& inGraphicData,
                  CollisionLocator& inCollisionLocator,
                  const TileExtent& inTileExtent)
    : TileMapBase(inGraphicData, inCollisionLocator, false)
    {
        tileExtent = inTileExtent;
    }
};

const nlohmann::json EMPTY_RESOURCE_DATA{
    nlohmann::json::parse(R"({"spriteSheets": [], "animations": [],
        "terrain": [], "floors": [], "walls": [], "objects": [],
        "entities": []})")};

/** Collision bounds that match the engine's default (ghost) entity. */
const BoundingBox GHOST_BOUNDS{{6.875f, 7.625f, 0}, {16.75f, 17.25f, 26.4f}};

/**
 * A test map with entities that move using the real movement physics.
 *
 * Note: Tile changes are picked up by the pathfinder's own nav grid. Don't use
 *       NavTestMap::update(), since its nav grid would consume the changes.
 */
struct PathfinderTestMap : public NavTestMap {
    entt::registry registry{};
    EntityLocator entityLocator{registry};
    GraphicDataBase graphicData{EMPTY_RESOURCE_DATA};
    ExtentTileMap tileMap{graphicData, collisionLocator, extent};
    Pathfinder pathfinder{registry, collisionLocator};
    EntityMover entityMover{registry, tileMap, entityLocator, collisionLocator};

    PathfinderTestMap() { entityLocator.setGridSize(extent); }

    /** Adds the tiles to the collision locator and builds the nav grid. */
    void buildMap()
    {
        syncTiles();
        pathfinder.init(extent);
    }

    /** Creates a moving entity, centered on the given tile. */
    entt::entity spawnEntity(const TilePosition& tilePosition,
                             const BoundingBox& modelBounds = GHOST_BOUNDS)
    {
        Vector3 center{tilePosition.getCenteredBottomPoint()};
        entt::entity entity{registry.create()};
        const Position& position{registry.emplace<Position>(entity, center)};
        registry.emplace<PreviousPosition>(entity).isInitialized = false;
        registry.emplace<Movement>(entity);
        registry.emplace<MovementModifiers>(entity);
        registry.emplace<Rotation>(entity);
        registry.emplace<Input>(entity);
        const Collision& collision{registry.emplace<Collision>(
            entity, modelBounds,
            Transforms::modelToWorldEntity(modelBounds, position))};
        const CollisionBitSets& collisionBitSets{
            registry.emplace<CollisionBitSets>(entity, entity, registry)};

        entityLocator.updateEntity(entity, position);
        collisionLocator.updateEntity(entity, collision.worldBounds,
                                      collisionBitSets.getCollisionLayers());
        return entity;
    }

    /** Runs one tick of pathfinding and movement. */
    void tick()
    {
        syncTiles();
        pathfinder.processPathFollowing();

        auto view{registry.view<Input, Position, PreviousPosition, Movement,
                                MovementModifiers, Rotation, Collision,
                                CollisionBitSets>()};
        for (auto [entity, input, position, previousPosition, movement,
                   movementMods, rotation, collision, collisionBitSets] :
             view.each()) {
            previousPosition = position;
            entityMover.moveEntity(
                {.entity{entity},
                 .inputStates{input.inputStates},
                 .position{position},
                 .previousPosition{previousPosition},
                 .movement{movement},
                 .movementMods{movementMods},
                 .rotation{rotation},
                 .collision{collision},
                 .collisionBitSets{collisionBitSets},
                 .deltaSeconds{SharedConfig::SIM_TICK_TIMESTEP_S}});
        }
    }

    /** Ticks until the entity's move status isn't Moving (or the tick limit
        is reached), and returns the final status. */
    MoveStatus tickUntilDone(entt::entity entity, int maxTicks = 1500)
    {
        for (int i{0}; i < maxTicks; ++i) {
            tick();
            if (pathfinder.getMoveStatus(entity) != MoveStatus::Moving) {
                break;
            }
        }
        return pathfinder.getMoveStatus(entity);
    }

    const Position& getPosition(entt::entity entity)
    {
        return registry.get<Position>(entity);
    }

    float horizontalDistance(const Vector3& a, const Vector3& b)
    {
        return std::sqrt(((a.x - b.x) * (a.x - b.x))
                         + ((a.y - b.y) * (a.y - b.y)));
    }
};
} // namespace

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder agent profiles")
{
    fillTerrain(0, Terrain::Height::Flat);
    buildMap();

    // The default entity: under a tile wide, under a third tall, and can
    // jump about 48 units (enough to climb a third).
    entt::entity smallEntity{spawnEntity({0, 0, 0})};
    NavAgentProfile smallAgent{pathfinder.calcAgentProfile(smallEntity)};
    REQUIRE(smallAgent.footprintTiles == 1);
    REQUIRE(smallAgent.heightThirds == 1);
    REQUIRE(smallAgent.climbThirds == 1);
    REQUIRE(smallAgent.maxDropThirds
            == SharedConfig::PATHFINDING_COMMON_MAX_DROP_THIRDS);

    // A wide, tall entity that jumps higher.
    entt::entity largeEntity{spawnEntity({5, 5, 0}, {{0, 0, 0}, {40, 40, 60}})};
    registry.get<MovementModifiers>(largeEntity).jumpImpulse = 300;
    NavAgentProfile largeAgent{pathfinder.calcAgentProfile(largeEntity)};
    REQUIRE(largeAgent.footprintTiles == 2);
    REQUIRE(largeAgent.heightThirds == 3);
    REQUIRE(largeAgent.climbThirds == 2);
}

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder moving to a position")
{
    fillTerrain(0, Terrain::Height::Flat);
    buildMap();

    entt::entity entity{spawnEntity({-5, -3, 0})};
    Vector3 target{TilePosition{6, 4, 0}.getCenteredBottomPoint()};
    REQUIRE(pathfinder.moveTo(entity, target));
    REQUIRE(pathfinder.getMoveStatus(entity) == MoveStatus::Moving);

    REQUIRE(tickUntilDone(entity) == MoveStatus::Arrived);
    REQUIRE(horizontalDistance(getPosition(entity), target)
            <= Pathfinder::DEFAULT_ACCEPTANCE_RADIUS);

    // Once it arrives, its inputs are released.
    REQUIRE(registry.get<Input>(entity).inputStates.none());

    // Entities without the required components can't be moved.
    entt::entity staticEntity{registry.create()};
    registry.emplace<Position>(staticEntity);
    REQUIRE(!(pathfinder.moveTo(staticEntity, target)));
}

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder only changes inputs as needed")
{
    fillTerrain(0, Terrain::Height::Flat);
    buildMap();

    // Count how many times the entity's inputs are updated (each update is
    // sent to nearby clients).
    int inputUpdateCount{0};
    struct Counter {
        int* count;
        void onUpdate(entt::registry&, entt::entity) { (*count)++; }
    } counter{&inputUpdateCount};
    registry.on_update<Input>().connect<&Counter::onUpdate>(counter);

    // A straight path should take one update to start moving, and one to
    // stop.
    entt::entity entity{spawnEntity({-10, 0, 0})};
    pathfinder.moveTo(entity, TilePosition{10, 0, 0}.getCenteredBottomPoint());
    REQUIRE(tickUntilDone(entity) == MoveStatus::Arrived);
    REQUIRE(inputUpdateCount == 2);

    registry.on_update<Input>().disconnect<&Counter::onUpdate>(counter);
}

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder going around walls")
{
    fillTerrain(0, Terrain::Height::Flat);

    // A wall across the map, with a gap near the edge.
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        if (y != 12) {
            addWall({1, y, 0}, Wall::Type::West);
        }
    }
    buildMap();

    entt::entity entity{spawnEntity({-3, 0, 0})};
    Vector3 target{TilePosition{4, 0, 0}.getCenteredBottomPoint()};
    pathfinder.moveTo(entity, target);
    REQUIRE(tickUntilDone(entity) == MoveStatus::Arrived);
    REQUIRE(horizontalDistance(getPosition(entity), target)
            <= Pathfinder::DEFAULT_ACCEPTANCE_RADIUS);
}

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder climbing and dropping")
{
    fillTerrain(0, Terrain::Height::Flat);

    // A ledge, a third of a tile tall, covering the east half of the map.
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        for (int x{2}; x <= extent.xMax(); ++x) {
            addTerrain({x, y, 0}, Terrain::Height::OneThird);
        }
    }
    buildMap();

    // Jump up onto the ledge.
    entt::entity entity{spawnEntity({-3, 0, 0})};
    Vector3 upperTarget{TilePosition{6, 0, 0}.getCenteredBottomPoint()};
    upperTarget.z = (TILE_HEIGHT / 3);
    pathfinder.moveTo(entity, upperTarget);
    REQUIRE(tickUntilDone(entity) == MoveStatus::Arrived);
    REQUIRE(getPosition(entity).z == Catch::Approx(TILE_HEIGHT / 3).margin(1));

    // Drop back down.
    Vector3 lowerTarget{TilePosition{-6, 0, 0}.getCenteredBottomPoint()};
    pathfinder.moveTo(entity, lowerTarget);
    REQUIRE(tickUntilDone(entity) == MoveStatus::Arrived);
    REQUIRE(getPosition(entity).z == Catch::Approx(0).margin(1));
}

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder unreachable targets")
{
    fillTerrain(0, Terrain::Height::Flat);

    // A box of walls around (5, 5).
    addWall({5, 5, 0}, Wall::Type::West);
    addWall({5, 5, 0}, Wall::Type::North);
    addWall({6, 5, 0}, Wall::Type::West);
    addWall({5, 6, 0}, Wall::Type::North);
    buildMap();

    entt::entity entity{spawnEntity({0, 0, 0})};
    pathfinder.moveTo(entity, TilePosition{5, 5, 0}.getCenteredBottomPoint());
    REQUIRE(tickUntilDone(entity) == MoveStatus::Unreachable);
    REQUIRE(registry.get<Input>(entity).inputStates.none());

    // Targets in mid-air are unreachable.
    Vector3 airTarget{TilePosition{-5, -5, 0}.getCenteredBottomPoint()};
    airTarget.z = TILE_HEIGHT * 2;
    pathfinder.moveTo(entity, airTarget);
    REQUIRE(tickUntilDone(entity) == MoveStatus::Unreachable);
}

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder following a moving entity")
{
    fillTerrain(0, Terrain::Height::Flat);
    buildMap();

    // The target only needs a position.
    entt::entity target{registry.create()};
    registry.emplace<Position>(target,
                               TilePosition{8, 0, 0}.getCenteredBottomPoint());

    entt::entity follower{spawnEntity({-8, 0, 0})};
    static constexpr float RANGE{TILE_WIDTH};
    pathfinder.moveTo(follower, target, RANGE);
    REQUIRE(tickUntilDone(follower) == MoveStatus::Arrived);
    REQUIRE(horizontalDistance(getPosition(follower), getPosition(target))
            <= RANGE);

    // Move the target away. The follower should start moving again, and
    // catch up.
    registry.get<Position>(target)
        = TilePosition{-10, 10, 0}.getCenteredBottomPoint();
    tick();
    REQUIRE(pathfinder.getMoveStatus(follower) == MoveStatus::Moving);
    REQUIRE(tickUntilDone(follower) == MoveStatus::Arrived);
    REQUIRE(horizontalDistance(getPosition(follower), getPosition(target))
            <= RANGE);

    // If the target is destroyed, it's unreachable.
    registry.destroy(target);
    tick();
    REQUIRE(pathfinder.getMoveStatus(follower) == MoveStatus::Unreachable);
}

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder large agents")
{
    fillTerrain(0, Terrain::Height::Flat);

    // A line of obstacles, with a 2-tile gap.
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        if ((y < 0) || (y > 1)) {
            addTileEntity({0, y, 0},
                          {{0, 0, 0}, {TILE_WIDTH, TILE_WIDTH, TILE_HEIGHT}});
        }
    }
    buildMap();

    entt::entity entity{spawnEntity({-6, 0, 0}, {{0, 0, 0}, {40, 40, 30}})};
    REQUIRE(pathfinder.calcAgentProfile(entity).footprintTiles == 2);
    Vector3 target{TilePosition{6, 1, 0}.getOriginPoint()};
    pathfinder.moveTo(entity, target);
    REQUIRE(tickUntilDone(entity) == MoveStatus::Arrived);
}

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder stopping")
{
    fillTerrain(0, Terrain::Height::Flat);
    buildMap();

    entt::entity entity{spawnEntity({0, 0, 0})};
    pathfinder.moveTo(entity, TilePosition{10, 0, 0}.getCenteredBottomPoint());
    for (int i{0}; i < 5; ++i) {
        tick();
    }
    REQUIRE(registry.get<Input>(entity).inputStates.any());

    pathfinder.stop(entity);
    REQUIRE(pathfinder.getMoveStatus(entity) == MoveStatus::None);
    REQUIRE(registry.get<Input>(entity).inputStates.none());
    REQUIRE(pathfinder.getRequestQueue().getPendingCount() == 0);
}

TEST_CASE_METHOD(PathfinderTestMap, "Pathfinder getting stuck")
{
    fillTerrain(0, Terrain::Height::Flat);
    buildMap();

    // A wall of player collision. The nav grid ignores it, but we make this
    // entity collide with it. It's too tall to jump over.
    // Note: We use the PlayerEntity layer since this entity isn't in it
    //       (EntityMover doesn't skip an entity's own collision).
    for (int y{extent.y}; y <= extent.yMax(); ++y) {
        addTileEntity({0, y, 0},
                      {{0, 0, 0}, {TILE_WIDTH, TILE_WIDTH, TILE_HEIGHT}},
                      CollisionLayerType::PlayerEntity);
    }
    entt::entity entity{spawnEntity({-4, 0, 0})};
    registry.get<CollisionBitSets>(entity).setCollisionMask(
        CollisionBitSets::DEFAULT_COLLISION_MASK
        | CollisionLayerType::PlayerEntity);

    // The entity tries jumping and repathing, then gives up.
    pathfinder.moveTo(entity, TilePosition{4, 0, 0}.getCenteredBottomPoint());
    REQUIRE(tickUntilDone(entity) == MoveStatus::Stuck);
    REQUIRE(registry.get<Input>(entity).inputStates.none());
}
