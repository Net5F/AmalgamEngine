#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * The types of item interactions that the engine provides.
 *
 * Note: Don't use this enum directly, use ItemInteractionType (it combines the 
 *       engine's and the project's interactions).
 */
enum class EngineItemInteractionType : Uint8
{
    NotSet,

    // Note: All items support Examine, UseOn (handled by CombineItems and 
    //       UseItemOnEntityRequest), and Destroy (handled by 
    //       InventoryDeleteItem).
    Examine,

    // We reserve values 0 - 49. The project can start at 50.
    PROJECT_START = 50
};

} // End namespace AM
