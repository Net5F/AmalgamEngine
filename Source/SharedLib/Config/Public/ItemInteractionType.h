#pragma once

// Use the project's ItemInteractionType, if one is provided.
#if defined(AM_OVERRIDE_DEFAULT_CONFIGS)
#include "Override/ItemInteractionType.h"
#else
#include "EngineItemInteractionType.h"
#include <SDL_stdinc.h>

namespace AM
{

/**
 * The types of interactions that a user may be able to perform on an item.
 */
enum class ItemInteractionType : Uint8
{
    // Engine interactions (copied here so we can use one strongly-typed enum).
    NotSet = static_cast<Uint8>(EngineItemInteractionType::NotSet),

    // Note: All items support Examine, UseOn (handled by CombineItems and 
    //       UseItemOnEntityRequest), and Destroy (handled by 
    //       InventoryDeleteItem).
    Examine = static_cast<Uint8>(EngineItemInteractionType::Examine),

    // Project interactions.
    //MyInteraction = static_cast<Uint8>(EngineItemInteractionType::PROJECT_START),
};

} // End namespace AM

#endif // defined(AM_OVERRIDE_CONFIG)
