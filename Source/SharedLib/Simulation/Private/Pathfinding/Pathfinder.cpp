#include "Pathfinder.h"
#include "CollisionLocator.h"
#include "Position.h"
#include "Movement.h"
#include "MovementModifiers.h"
#include "Collision.h"
#include "AMMath.h"
#include "tracy/Tracy.hpp"
#include "entt/entity/registry.hpp"
#include <algorithm>
#include <cmath>

namespace AM
{

Pathfinder::Pathfinder(entt::registry& inRegistry,
                       CollisionLocator& inCollisionLocator)
: registry{inRegistry}
, navGrid{inCollisionLocator}
, navRegions{navGrid}
, requestQueue{navGrid, &navRegions}
{
    // When a follower is destroyed, release its path request.
    registry.on_destroy<PathFollow>()
        .connect<&Pathfinder::onPathFollowDestroyed>(this);
}

Pathfinder::~Pathfinder()
{
    registry.on_destroy<PathFollow>()
        .disconnect<&Pathfinder::onPathFollowDestroyed>(this);
}

void Pathfinder::init(const TileExtent& mapTileExtent)
{
    navGrid.setGridSize(mapTileExtent);
    navGrid.build();
    navRegions.build();
}

void Pathfinder::processPathFollowing()
{
    ZoneScoped;

    // Update the nav grid to reflect any collision changes.
    navGrid.update();
    navRegions.update();

    // Check for arrivals and request any paths that are needed.
    auto view{registry.view<PathFollow>()};
    for (auto [entity, pathFollow] : view.each()) {
        planPath(entity, pathFollow);
    }

    // Search for paths, within this tick's budget.
    requestQueue.processRequests();

    // Pick up any found paths, and set inputs to follow them.
    for (auto [entity, pathFollow] : view.each()) {
        followPath(entity, pathFollow);
    }
}

bool Pathfinder::moveTo(entt::entity entity, const Vector3& targetPosition,
                        float acceptanceRadius)
{
    return startMoving(entity, entt::null, targetPosition, acceptanceRadius);
}

bool Pathfinder::moveTo(entt::entity entity, entt::entity targetEntity,
                        float acceptanceRadius)
{
    return startMoving(entity, targetEntity, {}, acceptanceRadius);
}

void Pathfinder::stop(entt::entity entity)
{
    if (registry.all_of<PathFollow>(entity)) {
        setInputs(entity, {});

        // Note: Our on_destroy listener releases the follower's request.
        registry.remove<PathFollow>(entity);
    }
}

MoveStatus Pathfinder::getMoveStatus(entt::entity entity) const
{
    if (const PathFollow* pathFollow{registry.try_get<PathFollow>(entity)}) {
        return pathFollow->status;
    }

    return MoveStatus::None;
}

const NavGrid& Pathfinder::getNavGrid() const
{
    return navGrid;
}

NavRequestQueue& Pathfinder::getRequestQueue()
{
    return requestQueue;
}

NavAgentProfile Pathfinder::calcAgentProfile(entt::entity entity) const
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float EPSILON{Math::WORLD_EPSILON};

    const auto [collision, movementMods]
        = registry.get<Collision, MovementModifiers>(entity);
    const BoundingBox& bounds{collision.modelBounds};

    // Round the agent's size up to whole tiles and thirds.
    float width{
        std::max(bounds.max.x - bounds.min.x, bounds.max.y - bounds.min.y)};
    float footprintTiles{std::ceil((width / TILE_WIDTH) - EPSILON)};
    float height{bounds.max.z - bounds.min.z};
    float heightThirds{std::ceil((height / NavGrid::THIRD_HEIGHT) - EPSILON)};

    // Agents can climb whatever they can jump onto.
    float climbThirds{std::floor(calcJumpHeight(movementMods.jumpImpulse)
                                 / NavGrid::THIRD_HEIGHT)};

    NavAgentProfile agent{};
    agent.footprintTiles = static_cast<Uint8>(
        std::clamp(footprintTiles, 1.f,
                   static_cast<float>(NavAgentProfile::MAX_FOOTPRINT_TILES)));
    agent.heightThirds
        = static_cast<Uint8>(std::clamp(heightThirds, 1.f, 255.f));
    agent.climbThirds = static_cast<Uint8>(std::clamp(climbThirds, 0.f, 255.f));
    agent.maxDropThirds = SharedConfig::PATHFINDING_COMMON_MAX_DROP_THIRDS;
    return agent;
}

std::optional<TilePosition> Pathfinder::findNavTile(const Vector3& position,
                                                    Uint8 footprintTiles) const
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    static constexpr float HALF_THIRD{NavGrid::THIRD_HEIGHT / 2.f};
    static constexpr float EPSILON{Math::WORLD_EPSILON};

    // Find the surface that the agent is standing on. Check its center column
    // first, then any other columns under its footprint (it may be hanging
    // over a ledge).
    const float halfWidth{(footprintTiles * TILE_WIDTH) / 2.f};
    const int centerX{static_cast<int>(std::floor(position.x / TILE_WIDTH))};
    const int centerY{static_cast<int>(std::floor(position.y / TILE_WIDTH))};
    const int minX{
        static_cast<int>(std::floor((position.x - halfWidth) / TILE_WIDTH))};
    const int maxX{static_cast<int>(
        std::floor((position.x + halfWidth - EPSILON) / TILE_WIDTH))};
    const int minY{
        static_cast<int>(std::floor((position.y - halfWidth) / TILE_WIDTH))};
    const int maxY{static_cast<int>(
        std::floor((position.y + halfWidth - EPSILON) / TILE_WIDTH))};

    auto findStandingSurface = [&](int x, int y) -> const NavSurface* {
        for (const NavSurface& surface : navGrid.getSurfaces(x, y)) {
            if (std::abs(navGrid.getWorldZ(surface.height) - position.z)
                <= HALF_THIRD) {
                return &surface;
            }
        }
        return nullptr;
    };

    int surfaceX{centerX};
    int surfaceY{centerY};
    const NavSurface* surface{findStandingSurface(centerX, centerY)};
    for (int y{minY}; (y <= maxY) && !surface; ++y) {
        for (int x{minX}; (x <= maxX) && !surface; ++x) {
            surface = findStandingSurface(x, y);
            surfaceX = x;
            surfaceY = y;
        }
    }
    if (!surface) {
        return std::nullopt;
    }

    // Small agents use the column they're standing on. Large agents use the
    // anchor of the footprint that's centered on them.
    int tileX{surfaceX};
    int tileY{surfaceY};
    if (footprintTiles > 1) {
        tileX = static_cast<int>(
            std::floor(((position.x - halfWidth) / TILE_WIDTH) + 0.5f));
        tileY = static_cast<int>(
            std::floor(((position.y - halfWidth) / TILE_WIDTH) + 0.5f));
    }

    // Use the tile layer that contains the surface.
    int tileZ{navGrid.getTileExtent().z
              + static_cast<int>(std::floor(surface->height / 3.f))};
    return TilePosition{tileX, tileY, tileZ};
}

bool Pathfinder::startMoving(entt::entity entity, entt::entity targetEntity,
                             const Vector3& targetPosition,
                             float acceptanceRadius)
{
    static constexpr float REPATH_DISTANCE{
        SharedConfig::PATHFINDING_REPATH_DISTANCE_TILES
        * SharedConfig::TILE_WORLD_WIDTH};

    if (!(registry.all_of<Input, Position, Movement, MovementModifiers,
                          Collision>(entity))) {
        return false;
    }

    // If the entity is already moving, update its target.
    if (PathFollow * pathFollow{registry.try_get<PathFollow>(entity)}) {
        // If the target changed, start moving towards it.
        // Note: Small changes to a target position are handled like a target
        //       entity moving (see planPath()), so callers can pass a
        //       slightly different position each tick.
        bool targetChanged{
            (pathFollow->targetEntity != targetEntity)
            || ((targetEntity == entt::null)
                && (calcHorizontalDistance(pathFollow->targetPosition,
                                           targetPosition)
                    > REPATH_DISTANCE))};
        pathFollow->targetEntity = targetEntity;
        pathFollow->targetPosition = targetPosition;
        pathFollow->acceptanceRadius = acceptanceRadius;
        if (targetChanged) {
            pathFollow->status = MoveStatus::Moving;
            pathFollow->shouldRepath = true;
        }
        return true;
    }

    PathFollow& pathFollow{registry.emplace<PathFollow>(entity)};
    pathFollow.targetEntity = targetEntity;
    pathFollow.targetPosition = targetPosition;
    pathFollow.acceptanceRadius = acceptanceRadius;
    pathFollow.agent = calcAgentProfile(entity);
    pathFollow.progressCheckPosition = registry.get<Position>(entity);
    return true;
}

void Pathfinder::planPath(entt::entity entity, PathFollow& pathFollow)
{
    static constexpr float TILE_HEIGHT{
        static_cast<float>(SharedConfig::TILE_WORLD_HEIGHT)};
    static constexpr float REPATH_DISTANCE{
        SharedConfig::PATHFINDING_REPATH_DISTANCE_TILES
        * SharedConfig::TILE_WORLD_WIDTH};

    if (pathFollow.ticksSinceRequest < SDL_MAX_UINT32) {
        pathFollow.ticksSinceRequest++;
    }
    if (pathFollow.status == MoveStatus::Stuck) {
        return;
    }

    // If the target entity is gone, there's nowhere to go.
    std::optional<Vector3> target{getTargetPosition(pathFollow)};
    if (!target) {
        if (pathFollow.status != MoveStatus::Unreachable) {
            setStatus(entity, pathFollow, MoveStatus::Unreachable);
        }
        return;
    }

    // If we're in range of the target, we've arrived.
    const Position& position{registry.get<Position>(entity)};
    if ((calcHorizontalDistance(position, *target)
         <= pathFollow.acceptanceRadius)
        && (std::abs(position.z - target->z) <= (TILE_HEIGHT / 2))) {
        if (pathFollow.status != MoveStatus::Arrived) {
            setStatus(entity, pathFollow, MoveStatus::Arrived);
        }
        return;
    }

    // If we arrived but the target moved out of range, start moving again.
    // If the target was unreachable, only try again if it moved.
    bool targetMoved{
        (calcHorizontalDistance(*target, pathFollow.pathGoal) > REPATH_DISTANCE)
        || (std::abs(target->z - pathFollow.pathGoal.z) > (TILE_HEIGHT / 2))};
    if (pathFollow.status == MoveStatus::Unreachable) {
        if (!targetMoved) {
            return;
        }
        pathFollow.status = MoveStatus::Moving;
    }
    else if (pathFollow.status == MoveStatus::Arrived) {
        pathFollow.status = MoveStatus::Moving;
        pathFollow.shouldRepath = true;
    }

    // If we don't need a new path (or it's too soon to ask), do nothing.
    bool hasPath{!(pathFollow.waypoints.empty())
                 || requestQueue.isValid(pathFollow.request)};
    bool gridChanged{!(pathFollow.waypoints.empty())
                     && (pathFollow.pathGridVersion != navGrid.getVersion())};
    if ((hasPath && !targetMoved && !gridChanged && !pathFollow.shouldRepath)
        || (pathFollow.ticksSinceRequest
            < SharedConfig::PATHFINDING_REPATH_INTERVAL_TICKS)
        || registry.get<Movement>(entity).isAirborne) {
        return;
    }

    // If the entity isn't standing on a surface (e.g. it's on top of an
    // object), we can't find a path from it.
    std::optional<TilePosition> startTile{
        findNavTile(position, pathFollow.agent.footprintTiles)};
    if (!startTile) {
        if (!hasPath) {
            setStatus(entity, pathFollow, MoveStatus::Stuck);
        }
        return;
    }

    // If the target isn't standing on a surface (e.g. it's jumping), wait for
    // it to land. Target positions in mid-air are unreachable.
    std::optional<TilePosition> goalTile{
        findNavTile(*target, pathFollow.agent.footprintTiles)};
    if (!goalTile) {
        if (pathFollow.targetEntity == entt::null) {
            setStatus(entity, pathFollow, MoveStatus::Unreachable);
        }
        return;
    }

    releaseRequest(pathFollow);
    pathFollow.request
        = requestQueue.request(*startTile, *goalTile, pathFollow.agent);
    pathFollow.pathGoal = *target;
    pathFollow.ticksSinceRequest = 0;
    pathFollow.shouldRepath = false;
}

void Pathfinder::followPath(entt::entity entity, PathFollow& pathFollow)
{
    static constexpr float TILE_WIDTH{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH)};
    // The min distance along an axis that we'll steer towards.
    static constexpr float MIN_STEERING_TOLERANCE{2};
    // How far before a ledge's edge we jump.
    static constexpr float JUMP_LEAD_DISTANCE{TILE_WIDTH / 4};

    if (pathFollow.status != MoveStatus::Moving) {
        return;
    }

    const Position& position{registry.get<Position>(entity)};

    // If a path request finished, pick up its result.
    if (requestQueue.isValid(pathFollow.request)) {
        NavSearch::Status requestStatus{
            requestQueue.getStatus(pathFollow.request)};
        if (requestStatus == NavSearch::Status::Found) {
            std::span<const NavWaypoint> waypoints{
                requestQueue.getWaypoints(pathFollow.request)};
            pathFollow.waypoints.assign(waypoints.begin(), waypoints.end());
            pathFollow.waypointIndex = 0;
            pathFollow.pathGridVersion = navGrid.getVersion();
            releaseRequest(pathFollow);

            // Give the new path a fresh chance to make progress.
            pathFollow.progressCheckPosition = position;
            pathFollow.ticksSinceProgressCheck = 0;
        }
        else if (requestStatus == NavSearch::Status::InvalidStart) {
            setStatus(entity, pathFollow, MoveStatus::Stuck);
            return;
        }
        else if ((requestStatus == NavSearch::Status::Unreachable)
                 || (requestStatus == NavSearch::Status::NodeLimitReached)) {
            setStatus(entity, pathFollow, MoveStatus::Unreachable);
            return;
        }
        // Else, it's still searching. Keep following the old path (if any).
    }

    // If we're still waiting for our first path, stand still.
    if (pathFollow.waypoints.empty()) {
        setInputs(entity, {});
        return;
    }

    // Skip past any waypoints that we've reached.
    // Note: The tolerance scales with speed, so fast entities don't overshoot
    //       and oscillate.
    const auto [movement, movementMods, input]
        = registry.get<Movement, MovementModifiers, Input>(entity);
    float stepDistance{static_cast<float>(movementMods.runSpeed
                                          * SharedConfig::SIM_TICK_TIMESTEP_S)};
    float steeringTolerance{std::max(MIN_STEERING_TOLERANCE, stepDistance)};
    std::vector<NavWaypoint>& waypoints{pathFollow.waypoints};
    while ((pathFollow.waypointIndex < waypoints.size())
           && hasReachedWaypoint(position, waypoints[pathFollow.waypointIndex],
                                 (steeringTolerance * 2))) {
        pathFollow.waypointIndex++;
    }

    // Steer towards the next waypoint. After the last one, steer towards the
    // target itself.
    Vector3 steeringTarget{pathFollow.pathGoal};
    const NavWaypoint* nextWaypoint{nullptr};
    if (pathFollow.waypointIndex < waypoints.size()) {
        nextWaypoint = &(waypoints[pathFollow.waypointIndex]);
        steeringTarget = nextWaypoint->position;
    }
    else if (std::optional<Vector3> target{getTargetPosition(pathFollow)}) {
        steeringTarget = *target;
    }
    Input::StateArr inputStates{
        calcSteeringInputs(position, steeringTarget, steeringTolerance)};

    // If we're approaching a ledge (or need to get unstuck), jump.
    // Note: Entities keep their horizontal velocity while in the air, so we
    //       only need to jump once. We only press jump if it was released
    //       last tick, since it must be released between jumps.
    if (!(movement.isAirborne) && !(input.inputStates[Input::Jump])) {
        float jumpDistance{((pathFollow.agent.footprintTiles * TILE_WIDTH) / 2)
                           + JUMP_LEAD_DISTANCE};
        bool isNearLedge{
            nextWaypoint && nextWaypoint->isJump
            && (calcHorizontalDistance(position, nextWaypoint->position)
                <= jumpDistance)};
        if (isNearLedge || pathFollow.shouldJump) {
            inputStates.set(Input::Jump);
            pathFollow.shouldJump = false;
        }
    }

    // Check if we're stuck. If we gave up, our inputs were released.
    updateProgress(entity, pathFollow, position);
    if (pathFollow.status != MoveStatus::Moving) {
        return;
    }

    setInputs(entity, inputStates);
}

std::optional<Vector3>
    Pathfinder::getTargetPosition(const PathFollow& pathFollow) const
{
    if (pathFollow.targetEntity == entt::null) {
        return pathFollow.targetPosition;
    }

    if (registry.valid(pathFollow.targetEntity)) {
        if (const Position* targetPosition{
                registry.try_get<Position>(pathFollow.targetEntity)}) {
            return *targetPosition;
        }
    }

    return std::nullopt;
}

void Pathfinder::setStatus(entt::entity entity, PathFollow& pathFollow,
                           MoveStatus status)
{
    pathFollow.status = status;
    if (status != MoveStatus::Moving) {
        releaseRequest(pathFollow);
        pathFollow.waypoints.clear();
        pathFollow.waypointIndex = 0;
        pathFollow.shouldJump = false;
        pathFollow.shouldRepath = false;
        pathFollow.failedProgressChecks = 0;
        setInputs(entity, {});
    }
}

void Pathfinder::releaseRequest(PathFollow& pathFollow)
{
    if (requestQueue.isValid(pathFollow.request)) {
        requestQueue.release(pathFollow.request);
    }
    pathFollow.request = {};
}

Input::StateArr Pathfinder::calcSteeringInputs(const Vector3& position,
                                               const Vector3& target,
                                               float tolerance) const
{
    // Move along each axis until we're within tolerance of the target.
    // Note: This moves diagonally until one axis lines up, then straight.
    //       Waypoints are always along cardinal or diagonal lines, so we
    //       follow the path exactly once we're on it.
    Input::StateArr inputStates{};
    float xDistance{target.x - position.x};
    float yDistance{target.y - position.y};
    if (xDistance > tolerance) {
        inputStates.set(Input::XUp);
    }
    else if (xDistance < -tolerance) {
        inputStates.set(Input::XDown);
    }
    if (yDistance > tolerance) {
        inputStates.set(Input::YUp);
    }
    else if (yDistance < -tolerance) {
        inputStates.set(Input::YDown);
    }

    return inputStates;
}

bool Pathfinder::hasReachedWaypoint(const Vector3& position,
                                    const NavWaypoint& waypoint,
                                    float tolerance) const
{
    // Note: If the waypoint is up a ledge, we must actually be up there.
    //       Waypoints below us count as reached once we're above them (we're
    //       falling to them).
    return (std::abs(waypoint.position.x - position.x) <= tolerance)
           && (std::abs(waypoint.position.y - position.y) <= tolerance)
           && (position.z
               >= (waypoint.position.z - (NavGrid::THIRD_HEIGHT / 2)));
}

void Pathfinder::updateProgress(entt::entity entity, PathFollow& pathFollow,
                                const Position& position)
{
    static constexpr float MIN_PROGRESS_DISTANCE{
        static_cast<float>(SharedConfig::TILE_WORLD_WIDTH) / 4.f};

    pathFollow.ticksSinceProgressCheck++;
    if (pathFollow.ticksSinceProgressCheck
        < SharedConfig::PATHFINDING_STUCK_TICKS) {
        return;
    }

    float movedSquaredDistance{
        position.squaredDistanceTo(pathFollow.progressCheckPosition)};
    pathFollow.progressCheckPosition = position;
    pathFollow.ticksSinceProgressCheck = 0;
    if (movedSquaredDistance
        >= (MIN_PROGRESS_DISTANCE * MIN_PROGRESS_DISTANCE)) {
        pathFollow.failedProgressChecks = 0;
        return;
    }

    // We're stuck. First try jumping, then a new path, then give up.
    pathFollow.failedProgressChecks++;
    if (pathFollow.failedProgressChecks == 1) {
        pathFollow.shouldJump = true;
    }
    else if (pathFollow.failedProgressChecks == 2) {
        pathFollow.shouldRepath = true;
    }
    else {
        setStatus(entity, pathFollow, MoveStatus::Stuck);
    }
}

void Pathfinder::setInputs(entt::entity entity,
                           const Input::StateArr& inputStates)
{
    if (registry.get<Input>(entity).inputStates != inputStates) {
        registry.patch<Input>(
            entity, [&](Input& input) { input.inputStates = inputStates; });
    }
}

float Pathfinder::calcJumpHeight(float jumpImpulse) const
{
    // Mirror MovementHelpers: on the jump tick, the velocity is set to the
    // impulse. Gravity is then applied every tick, including that one.
    static constexpr float GRAVITY{SharedConfig::FORCE_OF_GRAVITY};
    static constexpr float TIMESTEP{
        static_cast<float>(SharedConfig::SIM_TICK_TIMESTEP_S)};

    float height{0};
    float velocity{jumpImpulse - GRAVITY};
    while (velocity > 0) {
        height += (velocity * TIMESTEP);
        velocity -= GRAVITY;
    }

    return height;
}

float Pathfinder::calcHorizontalDistance(const Vector3& a,
                                         const Vector3& b) const
{
    float xDistance{a.x - b.x};
    float yDistance{a.y - b.y};
    return std::sqrt((xDistance * xDistance) + (yDistance * yDistance));
}

void Pathfinder::onPathFollowDestroyed(entt::registry& inRegistry,
                                       entt::entity entity)
{
    releaseRequest(inRegistry.get<PathFollow>(entity));
}

} // End namespace AM
