#pragma once

#include "ComponentTypeRegistryBase.h"

namespace AM
{
namespace Client
{

/**
 * See ComponentTypeRegistryBase class comment.
 */
class ComponentTypeRegistry : public ComponentTypeRegistryBase {
public:
    ComponentTypeRegistry(entt::registry& inRegistry);
};

} // End namespace Client
} // End namespace AM
