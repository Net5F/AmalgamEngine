#pragma once

#include "CastRequest.h"
#include "CastInfo.h"
#include "CastFailureType.h"
#include "EntityInteractionType.h"
#include "ItemInteractionType.h"
#include "SpellType.h"
#include "Vector3.h"
#include "QueuedEvents.h"
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
     * Passes the given cast to the associated callback, if one is registered.
     */
    void handleCast(const CastInfo& castInfo);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access entity data and castHelper. */
    World& world;
    /** Used to send CastFailed and CastStarted messages. */
    Network& network;
    const ItemData& itemData;
    const CastableData& castableData;

    EventQueue<CastRequest> castRequestQueue;
};

} // End namespace Server
} // End namespace AM
