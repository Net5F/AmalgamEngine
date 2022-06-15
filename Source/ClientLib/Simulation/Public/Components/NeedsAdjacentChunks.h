#pragma once

namespace AM
{
namespace Client
{
/**
 * Used to flag that we've just connected or just teleported and need to load
 * all adjacent tile map chunks.
 */
struct NeedsAdjacentChunks {
};

} // namespace Client
} // namespace AM
