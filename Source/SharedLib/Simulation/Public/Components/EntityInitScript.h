#pragma once

#include <string>

namespace AM
{
/**
 * An entity's initialization script. Init scripts allow builders to customize
 * an entity by adding components and interactions to it.
 *
 * Init scripts are stored on the server in this component. When a client
 * wants to edit an entity, we send them that entity's script so they don't
 * have to start from scratch.
 * When a client sends an updated init script for an entity, we:
 *   1. Destruct the entity.
 *   2. Re-create it and add back its default components.
 *   3. Run the new init script on it.
 *
 * Init scripts aren't used by client entities.
 */
struct EntityInitScript {
    /** Used as a "we should never hit this" cap on the length of the script
        string. */
    static constexpr std::size_t MAX_LENGTH{10000};

    /** The initialization script. */
    std::string script{};
};

template<typename S>
void serialize(S& serializer, EntityInitScript& initScript)
{
    serializer.text1b(initScript.script, EntityInitScript::MAX_LENGTH);
}

} // namespace AM
