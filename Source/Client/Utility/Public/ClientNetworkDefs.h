#pragma once

#include "NetworkDefs.h"
#include <SDL_stdinc.h>
#include <string>

/**
 * This file contains client-specific network definitions.
 */
namespace AM
{
class EntityUpdate;

namespace Client
{
//--------------------------------------------------------------------------
// Config
//--------------------------------------------------------------------------
// TODO: Eventually, move this config out into a file and read it into
//       a config class.
/** If true, the connection to the server will be mocked and we'll run without
    it. */
static constexpr bool RUN_OFFLINE = false;

/** Our best guess at a good starting place for the simulation's tick offset.
    Doesn't matter much since the server will quickly adjust us. */
static constexpr Sint8 INITIAL_TICK_OFFSET = 5;

//static const std::string SERVER_IP = "127.0.0.1";
 static const std::string SERVER_IP = "45.79.37.63";

static constexpr unsigned int SERVER_PORT = 41499;

/** How long we should wait before considering the server to be timed out. */
static constexpr double SERVER_TIMEOUT_S = NETWORK_TICK_TIMESTEP_S * 2;

//--------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------
/** The kind of update information present in a particular NpcUpdate. */
enum class NpcUpdateType {
    /** An update contains actual npc entity data. */
    Update,
    /** An implicit confirmation confirms all ticks up to the given tick. */
    ImplicitConfirmation,
    /** An explicit confirmation confirms 1 tick, with no particular tick given.
     */
    ExplicitConfirmation
};

/**
 * Represents a received NPC update message and/or any information we could
 * infer. Could contain data, or an implicit or explicit confirmation that no
 * changes occurred.
 */
struct NpcUpdateMessage {
    /** The type of information contained in this update. */
    NpcUpdateType updateType = NpcUpdateType::ExplicitConfirmation;
    /** If informationType == Update, contains the update message. */
    std::shared_ptr<const EntityUpdate> message = nullptr;
    /** If informationType == ImplicitConfirmation, contains the confirmed tick.
     */
    Uint32 tickNum = 0;
};

/** The result of trying to receive an NPC update message. */
struct NpcReceiveResult {
    NetworkResult result;
    /** message will be default if result != Success. */
    NpcUpdateMessage message;
};

} // End namespace Client
} // End namespace AM
