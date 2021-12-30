#pragma once

#include "MessageType.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <string>

namespace AM
{
/**
 * Contains static data (data that doesn't regularly change) for a single
 * entity.
 *
 * Used in EntityInit to tell a client to construct entities.
 */
struct EntityStaticData {
    /** Used as a "we should never hit this" cap on the length of the name
        string. Only checked in debug builds. */
    static constexpr unsigned int NAME_LENGTH = 50;

    /** The entity that this static data belongs to. */
    entt::entity entity{entt::null};

    /** The entity's name. */
//    std::string name{"Empty"};

    /** The numeric identifier for the entity's sprite. */
    int spriteNumericID{-1};
};

template<typename S>
void serialize(S& serializer, EntityStaticData& entityStaticData)
{
    serializer.value4b(entityStaticData.entity);
//    serializer.text1b(entityStaticData.name, EntityStaticData::NAME_LENGTH);
    serializer.value4b(entityStaticData.spriteNumericID);
}

} // End namespace AM
