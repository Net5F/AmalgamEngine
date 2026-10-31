#pragma once

#include "boost/mp11/algorithm.hpp"
#include "boost/mp11/set.hpp"
#include <SDL3/SDL_stdinc.h>
#include <type_traits>
#include <utility>

namespace AM
{
// Note: Persisted components are server-only.
namespace WorldServer
{

/**
 * Where a persisted component is saved when it's attached to a client entity.
 *
 * Non-client entities ignore this. All of their persisted components are
 * saved to the world's database.
 *
 * How to set up your component list, depending on the type of setup you want:
 *   Single server:
 *     Set everything to World (it's slightly more efficient to skip the 
 *     AccountServer -> WorldServer messages).
 *   Runescape-style (same character, multiple worlds):
 *     Set everything to Account, unless you want particular components to be
 *     per-world (e.g. player housing).
 *   Original WoW-style (world-locked characters):
 *     Set everything to World. Add logic so that if an account has no data in
 *     that world, you take them through a character creation flow.
 *
 * Note: Changing a component's ClientPersistence is a data migration. Its
 *       existing data will still be in the old location.
 */
enum class ClientPersistence {
    /** Follows the account to every world. Saved to the account server. */
    Account,
    /** Stays on this world. Saved to the world's database, keyed by account
        ID. Use this for data that references things in this world. */
    World,
    /** Not persisted for client entities. Rebuilt when the entity spawns. */
    None
};

template<typename ComponentT, Uint16 TypeIDValue, Uint16 VersionValue,
         ClientPersistence ClientPersistenceValue>
struct PersistedComponentEntry {
    using Component = ComponentT;
    static constexpr Uint16 TYPE_ID{TypeIDValue};
    static constexpr Uint16 VERSION{VersionValue};
    static constexpr ClientPersistence CLIENT_PERSISTENCE{
        ClientPersistenceValue};
};

/**
 * A component list that gives each component a stable ID.
 *
 * Component order in the template type list must be ordered by ascending ID,
 * but gaps may be present for deprecated IDs.
 *
 * IDs may be deprecated, but they must never be reused.
 */
template<typename... Entries>
struct PersistedComponentList {
    using EntryTypes = boost::mp11::mp_list<Entries...>;
    using ComponentTypes
        = boost::mp11::mp_list<typename Entries::Component...>;
    using TypeIDs = boost::mp11::mp_list<
        std::integral_constant<Uint16, Entries::TYPE_ID>...>;

    /** Gives the entry within this list for the given component type. */
    template<typename Component>
    using EntryFor = boost::mp11::mp_at_c<
        EntryTypes, boost::mp11::mp_find<ComponentTypes, Component>::value>;

    static_assert(boost::mp11::mp_is_set<ComponentTypes>::value,
                  "A persisted component type was registered more than once.");
    static_assert(boost::mp11::mp_is_set<TypeIDs>::value,
                  "A persisted component ID was registered more than once.");

    // We require the list to be manually sorted. This helps developers maintain
    // the "don't reuse IDs, add new entries to the bottom" rule.
    using SortedTypeIDs = boost::mp11::mp_sort<TypeIDs, boost::mp11::mp_less>;
    static_assert(std::is_same_v<TypeIDs, SortedTypeIDs>,
                  "Persisted component entries must be sorted by ID.");
};

} // End namespace WorldServer
} // End namespace AM
