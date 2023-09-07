#pragma once

#include "EngineMessageType.h"
#include "Position.h"
#include "AnimationState.h"
#include "ClientEntityInit.h"
#include "InitScriptResponse.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include <SDL_stdinc.h>

namespace AM
{
/**
 * Sent by the client to request that a dynamic object be created, or to 
 * request that an existing dynamic object be re-initialized.
 */
struct DynamicObjectInitRequest {
    // The EngineMessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::DynamicObjectInitRequest};

    /** If non-null, this message is a request to re-init this object. If null,
        this message is a request to create a new object. */
    entt::entity entity{entt::null};

    /** The new dynamic object's name. */
    std::string name{"Empty"};

    /** The position to place the object at. */
    Position position{};

    /** The object's animation info. */
    AnimationState animationState{};

    /** The script to run on this object after creation. */
    std::string initScript{};
};

template<typename S>
void serialize(S& serializer,
               DynamicObjectInitRequest& dynamicObjectInitRequest)
{
    serializer.value4b(dynamicObjectInitRequest.entity);
    serializer.text1b(dynamicObjectInitRequest.name,
                      ClientEntityInit::MAX_NAME_LENGTH);
    serializer.object(dynamicObjectInitRequest.position);
    serializer.object(dynamicObjectInitRequest.animationState);
    serializer.text1b(dynamicObjectInitRequest.initScript,
                      InitScriptResponse::MAX_SCRIPT_LENGTH);
}

} // End namespace AM
