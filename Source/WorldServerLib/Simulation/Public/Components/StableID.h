#pragma once

#include <string>

namespace AM
{
namespace WorldServer
{
/**
 * A world-unique ID that stays the same across server restarts.
 *
 * Entity IDs may change when an entity is re-initialized (e.g. edited in build
 * mode), so scripts that need to reference a specific NPC should give it a
 * stable ID and look it up using World::getEntityByStableID().
 *
 * Note: Don't add or modify this component directly. Use
 *       World::setStableID(), which enforces uniqueness and keeps
 *       World::stableIDMap in sync.
 */
struct StableID {
    /** The max length of a stable ID. */
    static constexpr std::size_t MAX_LENGTH{50};

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
