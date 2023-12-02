#pragma once

#include "IconID.h"
#include <string>

namespace AM
{
/**
 * Holds the data for a single icon from ResourceData.json.
 */
struct Icon {
    /** Unique display name, shown in the UI.  */
    std::string displayName{"Null"};

    /** The icon's unique string ID. Derived from displayName by replacing
        spaces with underscores and making everything lowercase.
        This ID will be consistent, and can be used for persistent state. */
    std::string stringID{"null"};

    /** This icon's unique numeric identifier.
        This value can be used safely at runtime, but shouldn't be used for
        persistent state since it may change when ResourceData.json is
        modified. */
    IconID numericID{NULL_ICON_ID};
};

} // namespace AM
