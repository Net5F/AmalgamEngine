#pragma once

#include "CastStarted.h"
#include "CastFailed.h"
#include "CastCooldownInit.h"
#include "Rotation.h"
#include "QueuedEvents.h"

namespace AM
{
class CastableData;
struct CastInfo;
struct Castable;

namespace Client
{
struct SimulationContext;
class Simulation;
class World;
class Network;
class GraphicData;
struct ClientCastState;

/**
 * Handles cast-related messages and all processing of ongoing casts.
 */
class CastSystem
{
public:
    CastSystem(const SimulationContext& inSimContext);

    /**
     * Handles received cast messages, and updates ongoing casts.
     */
    void processCasts();

private:
    /**
     * Processes any waiting casts that the UI queued through CastHelper.
     */
    void processUICasts();

    /**
     * Starts the given cast.
     */
    void handleCastStarted(const CastStarted& castStarted);

    /**
     * If the given message matches an ongoing cast, cancels it.
     */
    void handleCastFailed(const CastFailed& castFailed);

    /**
     * Initializes the player entity's CastCooldown component to the given 
     * state.
     */
    void handleCastCooldownInit(const CastCooldownInit& castCooldownInit);

    /**
     * Updates any ongoing casts, completing them as necessary.
     */
    void updateCasts();

    /**
     * Sets any entity state related to the cast being started.
     */
    void startCast(ClientCastState& castState);

    /**
     * Cancels the current cast, resetting cooldowns and sending error signals 
     * appropriately.
     */
    void cancelCast(ClientCastState& castState);

    /**
     * Finishes the given cast, either ending it or continuing on to the "cast 
     * complete" state.
     */
    void finishCast(ClientCastState& castState);

    /**
     * Returns the tick when the given entity should end the "cast complete" 
     * graphic of the given Castable, assuming they start on the current tick.
     *
     * Returns 0 if the given entity doesn't have a "cast complete" graphic.
     */
    Uint32 getCastCompleteEndTick(entt::entity entity,
                                  const Castable& castable);

    /**
     * Handles playing AV effects and creating AV entities for the given 
     * finished cast.
     */
    void playAVEffects(const CastInfo& castInfo);

    /** Used to get the current tick. */
    Simulation& simulation;
    /** Used to access entity data and castHelper. */
    World& world;
    /** Used to receive cast started/failed messages, and send cast requests. */
    Network& network;
    const GraphicData& graphicData;
    const CastableData& castableData;

    /** Used to send cast failure events to the UI. */
    entt::dispatcher& simEventDispatcher;

    EventQueue<CastStarted> castStartedQueue;
    EventQueue<CastFailed> castFailedQueue;
    EventQueue<CastCooldownInit> castCooldownInitQueue;
};

} // End namespace Client
} // End namespace AM
