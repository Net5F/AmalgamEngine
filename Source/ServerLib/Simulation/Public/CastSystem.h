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
#include "EventSorter.h"
#include "entt/fwd.hpp"
#include <unordered_map>
#include <functional>

namespace AM
{
class CastableData;
struct Castable;

namespace Server
{
class ItemData;
class Simulation;
class World;
class Network;
struct CastState;

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
     * Processes the given cast request.
     */
    void processCastRequest(const CastRequest& castRequest);

    /**
     * Updates any ongoing casts, canceling or completing them as necessary.
     */
    void updateCasts();

    /**
     * Sends a CastStarted message and sets any entity state related to the 
     * cast being started.
     */
    void startCast(CastState& castState);

    /**
     * Cancels the current cast, resetting cooldowns and sending error messages 
     * appropriately.
     */
    void cancelCast(CastState& castState);

    /**
     * Finishes the given cast.
     */
    void finishCast(CastState& castState);

    /**
     * Sends a CastStarted message to all nearby clients.
     */
    void sendCastStarted(CastState& castState);

    /**
     * Sends a CastFailed message to all nearby clients.
     */
    void sendCastFailed(CastState& castState, CastFailureType failureType);

    /**
     * Passes the given cast to the associated callback, if one is registered.
     */
    void handleCast(const CastInfo& castInfo);

    /**
     * Returns true if the caster is in range and in LoS of the target.
     *
     * Note: CastHelper validates casts before they start, so we only use this 
     *       before finishing a cast.
     */
    bool castIsValid(const CastInfo& castInfo, const Vector3& casterPosition);

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
    EventSorter<CastRequest> castRequestSorter;
};

} // End namespace Server
} // End namespace AM
