#pragma once

#include "EngineItemInteractionType.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * The types of interactions that a user may be able to perform on an item.
 *
 * Note: When you update the engine, you may need to add additional values
 *       here. Refer to the engine's default ItemInteractionType.h.
 */
enum class ItemInteractionType : Uint8 {
    // Engine interactions (copied here so we can use one strongly-typed enum).
    NotSet = static_cast<Uint8>(EngineItemInteractionType::NotSet),

    // Note: All items support Examine, Destroy (handled by
    //       InventoryDeleteItem), and UseOn (handled by CombineItems and 
    //       UseItemOnEntityRequest).
    UseOn = static_cast<Uint8>(EngineItemInteractionType::UseOn),
    Destroy = static_cast<Uint8>(EngineItemInteractionType::Destroy),
    Examine = static_cast<Uint8>(EngineItemInteractionType::Examine),

    // Project interactions.
    // MyInteraction =
    // static_cast<Uint8>(EngineItemInteractionType::PROJECT_START),
};

} // End namespace AM
