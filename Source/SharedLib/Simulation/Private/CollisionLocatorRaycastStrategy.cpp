#include "CollisionLocatorRaycastStrategy.h"

namespace AM
{

CollisionLocator::RaycastStrategyIntersectAny::RaycastStrategyIntersectAny(
    CollisionLocator& inCollisionLocator)
: hasIntersected{false}
, collisionLocator{inCollisionLocator}
{
}

bool CollisionLocator::RaycastStrategyIntersectAny::isDone() const
{ 
    return hasIntersected;
}

// Note: There's probably a good way to refactor this intersect code to share 
//       more between the 3 strategies, but it wasn't immediately obvious how 
//       to do so without sacrificing performance. Improvements are welcome.
void CollisionLocator::RaycastStrategyIntersectAny::intersectObjectsInCell(
    const Vector3& start, const Vector3& inverseRayDirection,
    const CellPosition& cellPosition, CollisionObjectTypeMask objectTypeMask,
    bool ignoreInsideHits)
{
    // If the line intersects any of this cell's terrain, return true.
    // Note: We ignore modelBounds and collisionEnabled on terrain, all 
    //       terrain gets generated collision. 
    if (CollisionObjectType::TileLayer & objectTypeMask) {
        TileExtent cellTileExtent{
            CellExtent{cellPosition.x, cellPosition.y, cellPosition.z, 1, 1, 1},
            SharedConfig::COLLISION_LOCATOR_CELL_WIDTH,
            SharedConfig::COLLISION_LOCATOR_CELL_HEIGHT};
        for (int z{cellTileExtent.z}; z <= cellTileExtent.zMax(); ++z) {
            for (int y{cellTileExtent.y}; y <= cellTileExtent.yMax(); ++y) {
                for (int x{cellTileExtent.x}; x <= cellTileExtent.xMax();
                     ++x) {
                    TilePosition tilePosition{x, y, z};
                    std::size_t linearizedIndex{
                        collisionLocator.linearizeTileIndex(tilePosition)};
                    Terrain::Value terrainValue{
                        collisionLocator.terrainGrid[linearizedIndex]};
                    if (terrainValue == EMPTY_TERRAIN) {
                        continue;
                    }

                    // Check for inside hits.
                    BoundingBox collisionVolume{Terrain::calcWorldBounds(
                        tilePosition, terrainValue)};
                    if (ignoreInsideHits
                        && collisionVolume.contains(start)) {
                        continue;
                    }

                    // Check if the line intersects the volume.
                    // Note: Since we want to bound to t==1, it's important for 
                    //       inverseRayDirection to not be normalized.
                    if (collisionVolume
                            .intersects(start, inverseRayDirection, 0.f, 1.f)
                            .didIntersect) {
                        hasIntersected = true;
                        return;
                    }
                }
            }
        }
    }

    // If the line intersects any of this cell's objects, return true.
    std::size_t linearizedIndex{
        collisionLocator.linearizeCellIndex(cellPosition)};
    std::vector<Uint16>& cell{
        collisionLocator.collisionGrid[linearizedIndex]};
    for (Uint16 collisionVolumeIndex : cell) {
        const CollisionInfo& collisionInfo{
            collisionLocator.collisionVolumes[collisionVolumeIndex]};

        // Check for masking and inside hits.
        bool isInMask{
            static_cast<bool>(collisionInfo.objectType & objectTypeMask)};
        const BoundingBox& collisionVolume{collisionInfo.collisionVolume};
        if (!isInMask
            || (ignoreInsideHits && collisionVolume.contains(start))) {
            continue;
        }

        // Check if the line intersects the volume.
        // Note: Since we want to bound to t==1, it's important for 
        //       inverseRayDirection to not be normalized.
        if (collisionVolume.intersects(start, inverseRayDirection, 0.f, 1.f)
                .didIntersect) {
            hasIntersected = true;
            return;
        }
    }
}

CollisionLocator::RaycastStrategyIntersectFirst::RaycastStrategyIntersectFirst(
    CollisionLocator& inCollisionLocator)
: hasIntersected{false}
, firstHitInfo{}
, collisionLocator{inCollisionLocator}
{
}

bool CollisionLocator::RaycastStrategyIntersectFirst::isDone() const
{ 
    return hasIntersected;
}

void CollisionLocator::RaycastStrategyIntersectFirst::intersectObjectsInCell(
    const Vector3& start, const Vector3& inverseRayDirection,
    const CellPosition& cellPosition, CollisionObjectTypeMask objectTypeMask,
    bool ignoreInsideHits)
{
    // We use terrainCollisionVolumes[0] as scratch space, and [1] for the 
    // earliest hit.
    collisionLocator.terrainCollisionVolumes.emplace_back(
        BoundingBox{}, CollisionObjectType::TileLayer);
    collisionLocator.terrainCollisionVolumes.emplace_back(
        BoundingBox{}, CollisionObjectType::TileLayer);

    // If the line intersects any of this cell's terrain, return true.
    // Note: We ignore modelBounds and collisionEnabled on terrain, all 
    //       terrain gets generated collision. 
    if (CollisionObjectType::TileLayer & objectTypeMask) {
        TileExtent cellTileExtent{
            CellExtent{cellPosition.x, cellPosition.y, cellPosition.z, 1, 1, 1},
            SharedConfig::COLLISION_LOCATOR_CELL_WIDTH,
            SharedConfig::COLLISION_LOCATOR_CELL_HEIGHT};
        for (int z{cellTileExtent.z}; z <= cellTileExtent.zMax(); ++z) {
            for (int y{cellTileExtent.y}; y <= cellTileExtent.yMax(); ++y) {
                for (int x{cellTileExtent.x}; x <= cellTileExtent.xMax();
                     ++x) {
                    TilePosition tilePosition{x, y, z};
                    std::size_t linearizedIndex{
                        collisionLocator.linearizeTileIndex(tilePosition)};
                    Terrain::Value terrainValue{
                        collisionLocator.terrainGrid[linearizedIndex]};
                    if (terrainValue == EMPTY_TERRAIN) {
                        continue;
                    }

                    // Check for inside hits.
                    BoundingBox& collisionVolume{
                        collisionLocator.terrainCollisionVolumes[0]
                            .collisionVolume};
                    collisionVolume
                        = Terrain::calcWorldBounds(tilePosition, terrainValue);
                    if (ignoreInsideHits
                        && collisionVolume.contains(start)) {
                        continue;
                    }

                    // Check if the line intersects the volume.
                    // Note: Since we want to bound to t==1, it's important for 
                    //       inverseRayDirection to not be normalized.
                    auto intersectReturn{collisionVolume.intersects(
                        start, inverseRayDirection, 0.f, 1.f)};
                    if (intersectReturn.didIntersect) {
                        hasIntersected = true;

                        // If this is the earliest hit, track it.
                        if (intersectReturn.tMin < firstHitInfo.hitT) {
                            collisionLocator.terrainCollisionVolumes[1]
                                .collisionVolume
                                = collisionVolume;
                            firstHitInfo.hitT = intersectReturn.tMin;
                            firstHitInfo.collisionInfo = &(
                                collisionLocator.terrainCollisionVolumes[1]);
                        }
                    }
                }
            }
        }
    }

    // If the line intersects any of this cell's objects, return true.
    std::size_t linearizedIndex{
        collisionLocator.linearizeCellIndex(cellPosition)};
    std::vector<Uint16>& cell{
        collisionLocator.collisionGrid[linearizedIndex]};
    for (Uint16 collisionVolumeIndex : cell) {
        const CollisionInfo& collisionInfo{
            collisionLocator.collisionVolumes[collisionVolumeIndex]};

        // Check for masking and inside hits.
        bool isInMask{
            static_cast<bool>(collisionInfo.objectType & objectTypeMask)};
        const BoundingBox& collisionVolume{collisionInfo.collisionVolume};
        if (!isInMask
            || (ignoreInsideHits && collisionVolume.contains(start))) {
            continue;
        }

        // Check if the line intersects the volume.
        // Note: Since we want to bound to t==1, it's important for 
        //       inverseRayDirection to not be normalized.
        auto intersectReturn{collisionVolume.intersects(
            start, inverseRayDirection, 0.f, 1.f)};
        if (intersectReturn.didIntersect) {
            hasIntersected = true;

            // If this is the earliest hit, track it.
            if (intersectReturn.tMin < firstHitInfo.hitT) {
                firstHitInfo.hitT = intersectReturn.tMin;
                firstHitInfo.collisionInfo = &collisionInfo;
            }
        }
    }
}

CollisionLocator::RaycastStrategyIntersectAll::RaycastStrategyIntersectAll(
    CollisionLocator& inCollisionLocator)
: collisionLocator{inCollisionLocator}
{
}

bool CollisionLocator::RaycastStrategyIntersectAll::isDone() const
{ 
    return !(collisionLocator.raycastReturnVector.empty());
}

void CollisionLocator::RaycastStrategyIntersectAll::intersectObjectsInCell(
    const Vector3& start, const Vector3& inverseRayDirection,
    const CellPosition& cellPosition, CollisionObjectTypeMask objectTypeMask,
    bool ignoreInsideHits)
{
    // We use terrainCollisionVolumes.back() as scratch space, then lock it in 
    // by pushing a new element when we intersect.
    collisionLocator.terrainCollisionVolumes.emplace_back(
        BoundingBox{}, CollisionObjectType::TileLayer);

    // If the line intersects any of this cell's terrain, return true.
    // Note: We ignore modelBounds and collisionEnabled on terrain, all 
    //       terrain gets generated collision. 
    if (CollisionObjectType::TileLayer & objectTypeMask) {
        TileExtent cellTileExtent{
            CellExtent{cellPosition.x, cellPosition.y, cellPosition.z, 1, 1, 1},
            SharedConfig::COLLISION_LOCATOR_CELL_WIDTH,
            SharedConfig::COLLISION_LOCATOR_CELL_HEIGHT};
        for (int z{cellTileExtent.z}; z <= cellTileExtent.zMax(); ++z) {
            for (int y{cellTileExtent.y}; y <= cellTileExtent.yMax(); ++y) {
                for (int x{cellTileExtent.x}; x <= cellTileExtent.xMax();
                     ++x) {
                    TilePosition tilePosition{x, y, z};
                    std::size_t linearizedIndex{
                        collisionLocator.linearizeTileIndex(tilePosition)};
                    Terrain::Value terrainValue{
                        collisionLocator.terrainGrid[linearizedIndex]};
                    if (terrainValue == EMPTY_TERRAIN) {
                        continue;
                    }

                    // Check for inside hits.
                    BoundingBox& collisionVolume{
                        collisionLocator.terrainCollisionVolumes.back()
                            .collisionVolume};
                    collisionVolume
                        = Terrain::calcWorldBounds(tilePosition, terrainValue);
                    if (ignoreInsideHits
                        && collisionVolume.contains(start)) {
                        continue;
                    }

                    // Check if the line intersects the volume.
                    // Note: Since we want to bound to t==1, it's important for 
                    //       inverseRayDirection to not be normalized.
                    auto intersectReturn{collisionVolume.intersects(
                        start, inverseRayDirection, 0.f, 1.f)};
                    if (intersectReturn.didIntersect) {
                        collisionLocator.raycastReturnVector.emplace_back(
                            intersectReturn.tMin,
                            &(collisionLocator.terrainCollisionVolumes.back()));

                        // Add an element for the next iteration.
                        collisionLocator.terrainCollisionVolumes.emplace_back(
                            BoundingBox{}, CollisionObjectType::TileLayer);
                    }
                }
            }
        }
    }

    // If the line intersects any of this cell's objects, return true.
    std::size_t linearizedIndex{
        collisionLocator.linearizeCellIndex(cellPosition)};
    std::vector<Uint16>& cell{
        collisionLocator.collisionGrid[linearizedIndex]};
    for (Uint16 collisionVolumeIndex : cell) {
        const CollisionInfo& collisionInfo{
            collisionLocator.collisionVolumes[collisionVolumeIndex]};

        // Check for masking and inside hits.
        bool isInMask{
            static_cast<bool>(collisionInfo.objectType & objectTypeMask)};
        const BoundingBox& collisionVolume{collisionInfo.collisionVolume};
        if (!isInMask
            || (ignoreInsideHits && collisionVolume.contains(start))) {
            continue;
        }

        // Check if the line intersects the volume.
        // Note: Since we want to bound to t==1, it's important for 
        //       inverseRayDirection to not be normalized.
        auto intersectReturn{collisionVolume.intersects(
            start, inverseRayDirection, 0.f, 1.f)};
        if (intersectReturn.didIntersect) {
            collisionLocator.raycastReturnVector.emplace_back(
                intersectReturn.tMin, &collisionInfo);
        }
    }
}

} // End namespace AM
