#pragma once

#include "PlayerMovementUpdate.h"

namespace AM
{
namespace Client
{
/**
 * Holds updated movement state for non-player entities.
 * 
 * This isn't an actual message that gets sent by the server, instead it gets
 * split out when we receive a ComponentUpdate with the relevant data.
 */
struct NpcMovementUpdate : PlayerMovementUpdate {};

} // End namespace Client
} // End namespace AM
