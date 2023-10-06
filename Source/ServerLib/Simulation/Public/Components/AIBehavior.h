#pragma once

#include "AILogic.h"
#include <memory>

namespace AM
{
namespace Server
{
/**
 * An entity's AI behavior.
 */
struct AIBehavior {
    /** A polymorphic pointer to a specific kind of AI logic. */
    std::unique_ptr<AILogic> logic{};
};

} // namespace Server
} // namespace AM
