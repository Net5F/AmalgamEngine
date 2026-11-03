#pragma once

#include <string>
#include <string_view>

namespace AM
{
namespace WorldServer
{
/**
 * A world-unique ID that stays the same across server restarts.
 *
 * Entity IDs may change when an entity is re-initialized, so scripts that need
 * to reference a specific NPC should give it a stable ID and look it up using
 * World::getEntityByStableID().
 *
 * Note: Don't add or modify this component directly. Use
 *       World::setStableID(), which enforces uniqueness and keeps
 *       World::stableIDMap in sync.
 */
struct StableID {
    /** The max length of a stable ID. */
    static constexpr std::size_t MAX_LENGTH{50};

    /** The prefix used by IDs from World::assignUniqueStableID().
        World::setStableID() rejects IDs with this prefix, so generated IDs
        can't collide with script-assigned IDs. */
    static constexpr std::string_view UNIQUE_PREFIX{"_gen_"};

    /** The ID. Always a derived string ID (see
        StringTools::deriveStringID()). */
    std::string value{};
};

template<typename S>
void serialize(S& serializer, StableID& stableID)
{
    serializer.text1b(stableID.value, StableID::MAX_LENGTH);
}

} // namespace WorldServer
} // namespace AM
