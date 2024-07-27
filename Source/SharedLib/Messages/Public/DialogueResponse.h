#pragma once

#include "EngineMessageType.h"
#include "DialogueEvent.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include "bitsery/ext/std_variant.h"
#include <SDL_stdinc.h>
#include <vector>
#include <string>

namespace AM
{

/**
 * Sent by the server in response to a Talk interaction or a dialogue choice 
 * selection.
 * 
 * Contains the response dialogue and choices that should be displayed to the 
 * client.
 */
struct DialogueResponse {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::DialogueResponse};

    /** Used as a "we should never hit this" cap on the container lengths. */
    static constexpr std::size_t MAX_DIALOGUE_EVENTS{200};
    static constexpr std::size_t MAX_CHOICES{50};
    static constexpr std::size_t MAX_CHOICE_TEXT_LENGTH{500};

    /** The entity that is talking (i.e. the entity that was interacted with). */
    entt::entity entity{entt::null};

    /** The index of this topic within the entity's Dialogue::topics. */
    Uint8 topicIndex{0};

    /** The dialogue events that comprise both the response to the previously 
        selected choice (if there was one), and the current topic. */
    std::vector<DialogueEvent> dialogueEvents{};

    struct Choice {
        /** This choice's index within the topic's Topic::Choices. */
        Uint8 index{0};
        /** The text to display for this choice. */
        std::string displayText{};
    };
    /** The available dialogue choices. */
    std::vector<Choice> choices{};
};

template<typename S>
void serialize(S& serializer, DialogueResponse::Choice& choice)
{
    serializer.value1b(choice.index);
    serializer.text1b(choice.displayText,
                      DialogueResponse::MAX_CHOICE_TEXT_LENGTH);
}

template<typename S>
void serialize(S& serializer, DialogueResponse& dialogueResponse)
{
    serializer.value4b(dialogueResponse.entity);
    serializer.value1b(dialogueResponse.topicIndex);
    serializer.container(
        dialogueResponse.dialogueEvents,
        DialogueResponse::MAX_DIALOGUE_EVENTS,
        [](S& serializer, DialogueEvent& event) {
            // Note: This calls serialize() for each type.
            serializer.ext(event, bitsery::ext::StdVariant{});
        });
    serializer.container(dialogueResponse.choices,
                         DialogueResponse::MAX_CHOICES);
}

} // End namespace AM
