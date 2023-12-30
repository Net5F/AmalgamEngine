#pragma once

#include <SDL_stdinc.h>

namespace AM
{

/** An item's numeric ID.
    Note: Once created, items can't be deleted. You can, however, repurpose an
          ID by updating that item's definition. */
using ItemID = Uint16;

/**
 * The ID of the "null item", or the ID used to indicate that an item is
 * not present.
 *
 * Note: Since the null ID is 0, you can do null checks like "if (itemID)".
 */
static constexpr ItemID NULL_ITEM_ID{0};

/** An item's version.
    Each time an item's definition is changed, its version gets incremented.
    Used by clients to tell if they have the latest item definition, or if they
    need to request it.*/
using ItemVersion = Uint16;

} // End namespace AM
