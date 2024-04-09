#pragma once

#include <string>

namespace AM
{
/**
 * An item's initialization script. Init scripts allow builders to customize
 * an item by adding properties and interactions to it.
 *
 * Init scripts are stored on the server. When a client wants to edit an item,
 * we send them that item's script so they don't have to start from scratch.
 * When a client sends an updated init script for an item, we:
 *   1. Reset the item to a default state.
 *   2. Run the new init script on it.
 */
struct ItemInitScript {
    /** Used as a "we should never hit this" cap on the length of the script
        string. */
    static constexpr std::size_t MAX_LENGTH{10000};

    /** The initialization script. */
    std::string script{};
};

template<typename S>
void serialize(S& serializer, ItemInitScript& initScript)
{
    serializer.text1b(initScript.script, ItemInitScript::MAX_LENGTH);
}

} // namespace AM
