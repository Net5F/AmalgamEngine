#pragma once

#include "EngineMessageType.h"
#include "Position.h"
#include "Rotation.h"
#include "ClientEntityInit.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <string>

namespace AM
{
/**
 * Sent by the server to tell a client when a dynamic object enters their area 
 * of interest.
 */
struct DynamicObjectInit {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::DynamicObjectInit};

    /** The tick that this update corresponds to. */
    Uint32 tickNum{0};

    /** The dynamic object's entity ID. */
    entt::entity entity{entt::null};

    /** The object's name. */
    std::string name{"Empty"};

    /** The object's world position. */
    Position position{};

    /** The object's rotation. */
    Rotation rotation{};

    /** The object's sprite set. */
    Uint16 spriteSetID{0};
};

template<typename S>
void serialize(S& serializer, DynamicObjectInit& dynamicObjectInit)
{
    serializer.value4b(dynamicObjectInit.tickNum);
    serializer.value4b(dynamicObjectInit.entity);
    serializer.text1b(dynamicObjectInit.name,
                      ClientEntityInit::MAX_NAME_LENGTH);
    serializer.object(dynamicObjectInit.position);
    serializer.object(dynamicObjectInit.rotation);
    serializer.value2b(dynamicObjectInit.spriteSetID);
}

} // End namespace AM
