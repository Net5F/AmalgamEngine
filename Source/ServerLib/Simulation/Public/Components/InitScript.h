#pragma once

#include <string>

namespace AM
{
namespace Server
{
/**
 * An entity's initialization script. Init scripts allow builders to customize 
 * an entity, by adding components and logic to it. 
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
struct InitScript {
    /** The initialization script. */
    std::string script{};
};

} // namespace Server
} // namespace AM
