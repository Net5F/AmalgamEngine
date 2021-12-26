#include "catch2/catch_all.hpp"
#include "EntityLocator.h"
#include "entt/entity/registry.hpp"
#include "Position.h"
#include "BoundingBox.h"
#include "Transforms.h"
#include "Log.h"
#include <vector>

using namespace AM;

TEST_CASE("TestEntityLocator")
{
    entt::registry registry;
    EntityLocator entityLocator(registry);

    // Calc the cell world width, since it's private in the EntityLocator.
    const float CELL_WORLD_WIDTH{SharedConfig::CELL_WIDTH * SharedConfig::TILE_WORLD_WIDTH};

    // Set grid size.
    const unsigned int GRID_X_LENGTH{32};
    const unsigned int GRID_Y_LENGTH{16};
    entityLocator.setGridSize(GRID_X_LENGTH, GRID_Y_LENGTH);

    // Model-space bounding box.
    const float HALF_TILE{SharedConfig::TILE_WORLD_WIDTH / 2.f};
    BoundingBox modelBounds{0, SharedConfig::TILE_WORLD_WIDTH, 0
        , SharedConfig::TILE_WORLD_WIDTH, 0, SharedConfig::TILE_WORLD_WIDTH};

    // Define the cylinder.
    Position cylinderCenter{CELL_WORLD_WIDTH, CELL_WORLD_WIDTH, 0};
    float HALF_CELL{CELL_WORLD_WIDTH / 2.f};
    unsigned int radius{static_cast<unsigned int>(HALF_CELL)};

    SECTION("Coarse cylinder - Single intersected cell")
    {
        entt::entity entity{registry.create()};

        // Touching 1 intersected cell outside the cylinder.
        Position position{0, 0, 0};
        BoundingBox boundingBox{Transforms::modelToWorld(modelBounds, position)};
        entityLocator.setEntityLocation(entity, boundingBox);

        std::vector<entt::entity>* returnVector{&(entityLocator.getEntitiesCoarse(
            cylinderCenter, radius))};

        REQUIRE(returnVector->size() == 1);
        REQUIRE(returnVector->at(0) == entity);

        // Touching 1 intersected cell inside the cylinder.
        position = {CELL_WORLD_WIDTH, CELL_WORLD_WIDTH, 0};
        boundingBox = Transforms::modelToWorld(modelBounds, position);
        entityLocator.setEntityLocation(entity, boundingBox);

        returnVector = &(entityLocator.getEntitiesCoarse(
            cylinderCenter, radius));

        REQUIRE(returnVector->size() == 1);
        REQUIRE(returnVector->at(0) == entity);
    }

    SECTION("Coarse cylinder - Touching multiple intersected cells")
    {
        entt::entity entity{registry.create()};

        // Touching 2 intersected cells outside the cylinder.
        Position position{0, (CELL_WORLD_WIDTH - HALF_TILE), 0};
        BoundingBox boundingBox{Transforms::modelToWorld(modelBounds, position)};
        entityLocator.setEntityLocation(entity, boundingBox);

        std::vector<entt::entity>* returnVector{&(entityLocator.getEntitiesCoarse(
            cylinderCenter, radius))};

        REQUIRE(returnVector->size() == 1);
        REQUIRE(returnVector->at(0) == entity);

        // Touching 2 intersected cells inside the cylinder.
        position = {CELL_WORLD_WIDTH, (CELL_WORLD_WIDTH - HALF_TILE), 0};
        boundingBox = Transforms::modelToWorld(modelBounds, position);
        entityLocator.setEntityLocation(entity, boundingBox);

        returnVector = &(entityLocator.getEntitiesCoarse(
            cylinderCenter, radius));

        REQUIRE(returnVector->size() == 1);
        REQUIRE(returnVector->at(0) == entity);
    }

    SECTION("Coarse cylinder - Half touching intersected cell")
    {
        entt::entity entity{registry.create()};

        // On the border of an intersected cell.
        Position position{((CELL_WORLD_WIDTH * 2) - HALF_TILE), 0, 0};
        BoundingBox boundingBox{Transforms::modelToWorld(modelBounds, position)};
        entityLocator.setEntityLocation(entity, boundingBox);

        std::vector<entt::entity> returnVector{entityLocator.getEntitiesCoarse(
            cylinderCenter, radius)};

        REQUIRE(returnVector.size() == 1);
        REQUIRE(returnVector.at(0) == entity);
    }

    SECTION("Coarse cylinder - Outside")
    {
        entt::entity entity{registry.create()};

        // Not touching any intersected cells.
        Position position{(CELL_WORLD_WIDTH * 3), 0, 0};
        BoundingBox boundingBox{Transforms::modelToWorld(modelBounds, position)};
        entityLocator.setEntityLocation(entity, boundingBox);

        std::vector<entt::entity> returnVector{entityLocator.getEntitiesCoarse(
            cylinderCenter, radius)};

        REQUIRE(returnVector.size() == 0);
    }

    SECTION("Coarse cylinder - 2 out, 2 in")
    {
        // 2 out, 2 in.
        // Outside any intersected cells.
        entt::entity entity{registry.create()};
        Position position{(CELL_WORLD_WIDTH * 3), 0, 0};
        BoundingBox boundingBox{Transforms::modelToWorld(modelBounds, position)};
        entityLocator.setEntityLocation(entity, boundingBox);

        // Outside any intersected cells.
        entt::entity entity2{registry.create()};
        position = {CELL_WORLD_WIDTH, (CELL_WORLD_WIDTH * 3), 0};
        boundingBox = Transforms::modelToWorld(modelBounds, position);
        entityLocator.setEntityLocation(entity2, boundingBox);

        // Inside intersected cell, outside cylinder.
        entt::entity entity3{registry.create()};
        position = {HALF_TILE, HALF_TILE, 0};
        boundingBox = Transforms::modelToWorld(modelBounds, position);
        entityLocator.setEntityLocation(entity3, boundingBox);

        // Inside cylinder.
        entt::entity entity4{registry.create()};
        position = {CELL_WORLD_WIDTH, CELL_WORLD_WIDTH, 0};
        boundingBox = Transforms::modelToWorld(modelBounds, position);
        entityLocator.setEntityLocation(entity4, boundingBox);

        std::vector<entt::entity> returnVector{entityLocator.getEntitiesCoarse(
            cylinderCenter, radius)};

        REQUIRE(returnVector.size() == 2);
        REQUIRE(((returnVector.at(0) == entity3)
                || (returnVector.at(0) == entity4)));
        REQUIRE(((returnVector.at(1) == entity3)
                || (returnVector.at(1) == entity4)));
    }

    SECTION("Fine cylinder - Single cell inside")
    {
        entt::entity entity{registry.create()};

        // Single cell inside the cylinder.
        Position& position{registry.emplace<Position>(entity,
            (CELL_WORLD_WIDTH - SharedConfig::TILE_WORLD_WIDTH)
            , (CELL_WORLD_WIDTH - SharedConfig::TILE_WORLD_WIDTH), 0.f)};
        BoundingBox& boundingBox{registry.emplace<BoundingBox>(entity
            , Transforms::modelToWorld(modelBounds, position))};
        entityLocator.setEntityLocation(entity, boundingBox);

        std::vector<entt::entity> returnVector{entityLocator.getEntitiesFine(
            cylinderCenter, radius)};

        REQUIRE(returnVector.size() == 1);
        REQUIRE(returnVector.at(0) == entity);
    }

    SECTION("Fine cylinder - Touching multiple cells")
    {
        entt::entity entity{registry.create()};

        // Touching 4 cells inside the cylinder.
        Position& position{registry.emplace<Position>(entity,
            (CELL_WORLD_WIDTH - HALF_TILE)
            , (CELL_WORLD_WIDTH - HALF_TILE), 0.f)};
        BoundingBox& boundingBox{registry.emplace<BoundingBox>(entity
            , Transforms::modelToWorld(modelBounds, position))};
        entityLocator.setEntityLocation(entity, boundingBox);

        std::vector<entt::entity> returnVector{entityLocator.getEntitiesFine(
            cylinderCenter, radius)};

        REQUIRE(returnVector.size() == 1);
        REQUIRE(returnVector.at(0) == entity);
    }

    SECTION("Fine cylinder - On border")
    {
        entt::entity entity{registry.create()};

        // On the border of the cylinder.
        // Note: This is right on the edge (same value), which still counts
        //       as being inside.
        Position& position{registry.emplace<Position>(entity,
            (CELL_WORLD_WIDTH + HALF_CELL)
            , (CELL_WORLD_WIDTH - HALF_TILE), 0.f)};
        BoundingBox& boundingBox{registry.emplace<BoundingBox>(entity
            , Transforms::modelToWorld(modelBounds, position))};
        entityLocator.setEntityLocation(entity, boundingBox);

        std::vector<entt::entity> returnVector{entityLocator.getEntitiesFine(
            cylinderCenter, radius)};

        REQUIRE(returnVector.size() == 1);
        REQUIRE(returnVector.at(0) == entity);
    }

    SECTION("Fine cylinder - Outside")
    {
        entt::entity entity{registry.create()};

        // Inside an intersected cell, but outside the cylinder.
        Position& position{registry.emplace<Position>(entity,
            0.f, 0.f, 0.f)};
        BoundingBox& boundingBox{registry.emplace<BoundingBox>(entity
            , Transforms::modelToWorld(modelBounds, position))};
        entityLocator.setEntityLocation(entity, boundingBox);

        std::vector<entt::entity> returnVector{entityLocator.getEntitiesFine(
            cylinderCenter, radius)};

        REQUIRE(returnVector.size() == 0);
    }

    SECTION("Fine cylinder - 2 out, 2 in")
    {
        // 2 out, 2 in.
        // Inside an intersected cell, but outside the cylinder.
        entt::entity entity{registry.create()};
        Position& position{registry.emplace<Position>(entity,
            0.f, 0.f, 0.f)};
        BoundingBox& boundingBox{registry.emplace<BoundingBox>(entity
            , Transforms::modelToWorld(modelBounds, position))};
        entityLocator.setEntityLocation(entity, boundingBox);

        // Outside any intersected cells.
        entt::entity entity2{registry.create()};
        Position& position2{registry.emplace<Position>(entity2,
            (CELL_WORLD_WIDTH * 3), 0.f, 0.f)};
        BoundingBox& boundingBox2{registry.emplace<BoundingBox>(entity2
            , Transforms::modelToWorld(modelBounds, position2))};
        entityLocator.setEntityLocation(entity2, boundingBox2);

        // Inside the cylinder, touching a single intersected cell.
        entt::entity entity3{registry.create()};
        Position& position3{registry.emplace<Position>(entity3,
            CELL_WORLD_WIDTH, CELL_WORLD_WIDTH, 0.f)};
        BoundingBox& boundingBox3{registry.emplace<BoundingBox>(entity3
            , Transforms::modelToWorld(modelBounds, position3))};
        entityLocator.setEntityLocation(entity3, boundingBox3);

        // Inside the cylinder, touching 2 intersected cells.
        entt::entity entity4{registry.create()};
        Position& position4{registry.emplace<Position>(entity4,
            CELL_WORLD_WIDTH, (CELL_WORLD_WIDTH - HALF_TILE), 0.f)};
        BoundingBox& boundingBox4{registry.emplace<BoundingBox>(entity4
            , Transforms::modelToWorld(modelBounds, position4))};
        entityLocator.setEntityLocation(entity4, boundingBox4);

        std::vector<entt::entity> returnVector{entityLocator.getEntitiesFine(
            cylinderCenter, radius)};

        REQUIRE(returnVector.size() == 2);
        REQUIRE(((returnVector.at(0) == entity3)
                || (returnVector.at(0) == entity4)));
        REQUIRE(((returnVector.at(1) == entity3)
                || (returnVector.at(1) == entity4)));
    }
}
