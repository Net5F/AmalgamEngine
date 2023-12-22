#pragma once

#include <string>

namespace AM
{
namespace Server
{
/**
 * An entity's script to run when a particular item is used on it.
 * 
 * Handler scripts are written as part of an entity's init script, through the 
 * addItemHandler() lua function. 
 */
struct EntityItemHandlerScript {
    /** Used as a "we should never hit this" cap on the length of the script
        string. Only checked in debug builds. */
    static constexpr std::size_t MAX_LENGTH{10000};

    /** The item handler script. */
    std::string script{};
};

template<typename S>
void serialize(S& serializer, EntityItemHandlerScript& itemHandlerScript)
{
    serializer.text1b(itemHandlerScript.script,
                      EntityItemHandlerScript::MAX_LENGTH);
}

} // namespace Server
} // namespace AM
