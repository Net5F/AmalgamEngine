#pragma once

#include "CastRequest.h"
#include "CastInfo.h"
#include "CastFailureType.h"
#include "EntityInteractionType.h"
#include "ItemInteractionType.h"
#include "SpellType.h"
#include "Vector3.h"
#include "EnttObserver.h"
#include "QueuedEvents.h"
#include "entt/fwd.hpp"
#include <unordered_map>
#include <functional>

namespace AM
{
class CastableData;
struct Castable;
struct CastState;

namespace Server
{
class ItemData;
class Simulation;
class World;
class Network;

/**
 * Handles cast requests and all processing of ongoing casts.
 */
class CastSystem
{
public:
    CastSystem(Simulation& inSimulation, Network& inNetwork,
               const ItemData& inItemData, const CastableData& inCastableData);

    /**
     * Sends initial cast cooldown state to newly-logged-on clients.
     */
    void sendCastCooldownInits();

    /**
     * Processes cast requests, updates ongoing casts, cancels casts as 
     * necessary, and passes completed casts to registered handlers.
     */
    void processCasts();

private:
    /**
     * Processes all waiting cast request messages.
     */
    void processCastRequests();

    /**
     * Updates any ongoing casts, canceling or completing them as necessary.
     */
    void updateCasts();

    /**
     * Sends a CastStarted message and sets the given cast's end tick.
     */
    void startCast(CastState& castState);

    /**
     * Sends a CastStarted message to all nearby clients.
     */
    void sendCastStarted(CastState& castState);

    /**
     * Passes the given cast to the associated callback, if one is registered.
     */
    void handleCast(const CastInfo& castInfo);

    /**
     * Initializes any CastCooldown components on loaded entities.
     */
    void onCastCooldownConstructed(entt::registry& registry,
                                   entt::entity entity);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access entity data and castHelper. */
    World& world;
    /** Used to send CastFailed and CastStarted messages. */
    Network& network;
    const ItemData& itemData;
    const CastableData& castableData;

    /** Observes player CastCooldown component creation so we can send the 
        initial cast cooldown state to a newly-logged-on player. */
    EnttObserver playerCastCooldownObserver;

    EventQueue<CastRequest> castRequestQueue;
};

} // End namespace Server
} // End namespace AM
