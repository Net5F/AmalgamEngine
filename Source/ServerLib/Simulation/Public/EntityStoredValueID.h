#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/**
 * An entity stored value's numeric ID.
 * Note: Global stored values are handled separately. This file only applies 
 *       to values stored in an entity's StoredValues component.
 *
 * These IDs are created when a new string ID is first used, and are never 
 * deleted.
 * Note: If we ever care to reclaim these values, we can write a function 
 *       that iterates the StoredValues components and removes any string IDs 
 *       from World::storedValueIDMap that aren't used.
 */
using EntityStoredValueID = Uint16;

/**
 * The ID of the "null stored value", or the ID used to indicate that a  
 * value is not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (valueID)".
 */
static constexpr EntityStoredValueID NULL_ENTITY_STORED_VALUE_ID{0};

} // End namespace AM
