#pragma once

#include "EngineMessageType.h"
#include "entt/fwd.hpp"
#include "entt/entity/entity.hpp"
#include "bitsery/ext/std_variant.h"
#include <SDL_stdinc.h>
#include <variant>
#include <vector>
#include <string>

namespace AM
{

/**
 * Sent by the server to provide a client with a new section (topic) of an 
 * entity's dialogue tree.
 */
struct DialogueTopicResponse {
    // The MessageType enum value that this message corresponds to.
    // Declares this struct as a message that the Network can send and receive.
    static constexpr EngineMessageType MESSAGE_TYPE{
        EngineMessageType::DialogueTopicResponse};

    /** Used as a "we should never hit this" cap on the container lengths. */
    static constexpr std::size_t MAX_DIALOGUE_EVENTS{200};
    static constexpr std::size_t MAX_CHOICES{50};
    static constexpr std::size_t MAX_SAY_TEXT_LENGTH{500};
    static constexpr std::size_t MAX_CHOICE_TEXT_LENGTH{500};

    /** The entity that is talking. */
    entt::entity entity{entt::null};

    /** The index of this topic within the entity's Dialogue::topics. */
    Uint8 topicIndex{0};

    /** A dialogue event. strings come from the say() command (a line of text 
        to display). floats come from the wait() command (an amount of time to 
        wait before proceeding). */
    using DialogueEvent = std::variant<std::string, float>;
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
    std::vector<Choice> choices;
};

template<typename S>
void serialize(S& serializer, DialogueTopicResponse::Choice& choice)
{
    serializer.value1b(choice.index);
    serializer.text1b(choice.displayText,
                      DialogueTopicResponse::MAX_CHOICE_TEXT_LENGTH);
}

template<typename S>
void serialize(S& serializer, DialogueTopicResponse& dialogueTopicResponse)
{
    serializer.value4b(dialogueTopicResponse.entity);
    serializer.value1b(dialogueTopicResponse.topicIndex);
    serializer.container(
        dialogueTopicResponse.dialogueEvents,
        DialogueTopicResponse::MAX_DIALOGUE_EVENTS,
        // Note: It's messy, but we choose to handle the variant here to avoid 
        //       putting a serialize(variant<string, float>) in the top-level  
        //       namespace (which might cause conflicts in the future).
        [](S& serializer, DialogueTopicResponse::DialogueEvent& event) {
            serializer.ext(
                event, bitsery::ext::StdVariant{
                           [](S& serializer, std::string& sayText) {
                               serializer.text1b(
                                   sayText,
                                   DialogueTopicResponse::MAX_SAY_TEXT_LENGTH);
                           },
                           [](S& serializer, float& waitTime) {
                               serializer.value4b(waitTime);
                           }});
        });
    serializer.container(dialogueTopicResponse.choices,
                         DialogueTopicResponse::MAX_CHOICES);
}

} // End namespace AM
