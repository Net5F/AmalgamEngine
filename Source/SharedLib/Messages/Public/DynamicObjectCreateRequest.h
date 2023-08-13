#pragma once

#include "EngineMessageType.h"
#include "Position.h"
#include "ClientEntityInit.h"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by the client to request that a dynamic object be created.
 */
struct DynamicObjectCreateRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::DynamicObjectCreateRequest};

    /** The new dynamic object's name. */
    std::string name{"Empty"};

    /** The position to place the object at. */
    Position position{};

    /** The object's rotation. */
    Rotation rotation{};

    /** The object's sprite set. */
    Uint16 spriteSetID{0};
};

template<typename S>
void serialize(S& serializer,
               DynamicObjectCreateRequest& dynamicObjectCreateRequest)
{
    serializer.text1b(dynamicObjectCreateRequest.name,
                      ClientEntityInit::NAME_LENGTH);
    serializer.object(dynamicObjectCreateRequest.position);
    serializer.object(dynamicObjectCreateRequest.rotation);
    serializer.value2b(dynamicObjectCreateRequest.spriteSetID);
}

} // End namespace AM
