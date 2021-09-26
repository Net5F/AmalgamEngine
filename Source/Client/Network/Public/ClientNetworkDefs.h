#pragma once

#include "EntityUpdate.h"
#include "NetworkDefs.h"
#include <SDL2/SDL_stdinc.h>
#include <memory>

/**
 * This file contains client-specific network definitions.
 */
namespace AM
{
class EntityUpdate;

namespace Client
{
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

} // End namespace Client
} // End namespace AM
