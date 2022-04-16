#include "catch2/catch_all.hpp"
#include "Position.h"
#include "BoundingBox.h"
#include "TileExtent.h"
#include "Log.h"

using namespace AM;

TEST_CASE("TestBoundingBox")
{
    SECTION("Intersects cylinder")
    {
        // Centered on the origin.
        Position position{0, 0, 0};
        unsigned int radius{256};

        // Fully inside the cylinder.
        BoundingBox box1{1, 6, 3, 8, 0, 1};
        REQUIRE(box1.intersects(position, radius));

        // Corner inside the cylinder, center outside.
        BoundingBox box2{0, 10, 255, 265, 0, 1};
        REQUIRE(box2.intersects(position, radius));

        // Center inside the cylinder, corner outside.
        BoundingBox box3{0, 10, 250, 260, 0, 1};
        REQUIRE(box3.intersects(position, radius));

        // Edge shared with cylinder.
        BoundingBox box4{256, 266, 0, 10, 0, 1};
        REQUIRE(box4.intersects(position, radius));

        // Fully outside the cylinder.
        BoundingBox box5{300, 310, 300, 310, 0, 1};
        REQUIRE(!(box5.intersects(position, radius)));
    }

    SECTION("Intersects tile extent")
    {
        TileExtent tileExtent{0, 0, 1, 1};

        // Fully inside the extent.
        BoundingBox box1{10, 15, 10, 15, 0, 1};
        REQUIRE(box1.intersects(tileExtent));

        // Partially inside the extent.
        BoundingBox box2{30, 35, 30, 35, 0, 1};
        REQUIRE(box2.intersects(tileExtent));

        // Edge shared with extent.
        BoundingBox box3{32, 37, 30, 35, 0, 1};
        REQUIRE(box3.intersects(tileExtent));

        // Fully outside the extent.
        BoundingBox box4{33, 38, 30, 35, 0, 1};
        REQUIRE(!(box4.intersects(tileExtent)));
    }

    SECTION("As tile extent")
    {
        const float TILE_WIDTH{SharedConfig::TILE_WORLD_WIDTH};

        BoundingBox box{(TILE_WIDTH * 1.25),
                        (TILE_WIDTH * 4),
                        (TILE_WIDTH * 0.75),
                        (TILE_WIDTH * 4),
                        0,
                        TILE_WIDTH};
        TileExtent tileExtent{box.asTileExtent()};

        REQUIRE(tileExtent.x == 1);
        REQUIRE(tileExtent.xLength == 3);
        REQUIRE(tileExtent.y == 0);
        REQUIRE(tileExtent.yLength == 4);
    }

    SECTION("Get 3d center")
    {
        BoundingBox box{300, 310, 300, 310, 0, 10};
        Position center{box.get3dCenter()};

        REQUIRE(center.x == 305);
        REQUIRE(center.y == 305);
        REQUIRE(center.z == 5);
    }
}
