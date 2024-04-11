#pragma once

#include "bitsery/ext/std_map.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace AM
{
namespace Server
{
/**
 * Holds the list of topics that make up an entity's dialogue tree.
 *
 * Instead of an actual tree structure, we use a stateless look-up table.
 * When a client requests to begin a dialogue with an entity, the server sends 
 * them the first topic in that entity's topics vector. The client can then 
 * request that a choice be selected, which may result in a new topic being sent.
 */
struct Dialogue {
    /** Used as a "we should never hit this" cap on the container lengths. */
    static constexpr std::size_t MAX_CHOICE_CONDITION_SCRIPT_LENGTH{500};
    static constexpr std::size_t MAX_CHOICE_DISPLAY_TEXT_LENGTH{200};
    static constexpr std::size_t MAX_CHOICE_ACTION_SCRIPT_LENGTH{1000};
    static constexpr std::size_t MAX_TOPIC_SCRIPT_LENGTH{1500};
    static constexpr std::size_t MAX_CHOICES{50};
    static constexpr std::size_t MAX_TOPICS{100};
    static constexpr std::size_t MAX_TOPIC_NAME_LENGTH{100};

    /**
     * A choice that the player can make.
     */
    struct Choice {
        /** If non-empty, contains a condition script that must be ran against 
            the player entity to check if they may access this choice.
            Condition scripts will have "r =" prepended to them before running, 
            and must evaluate to a boolean value. */
        std::string conditionScript{};

        /** The text that will be displayed for this choice. */
        std::string displayText{};

        /** The script to run if this choice is successfully selected. */
        std::string actionScript{};
    };

    /**
     * A dialogue topic.
     */
    struct Topic {
        /** This topic's name. */
        std::string name{};

        /** The script to run when this topic is entered. */
        std::string topicScript{};

        /** The choices that will be displayed for the player to select.
            Choices are added to this vector based on their order in the choice
            script. */
        std::vector<Choice> choices{};
    };

    /** A map of topic names -> their index in the topics vector. */
    std::unordered_map<std::string, Uint8> topicIndices{};

    /** The available dialogue topics.
        Topics are added to this vector based on their order in the entity's 
        init script. The first topic will be the one sent in response to the 
        Talk interaction. The rest are only reachable using setNextTopic().
        Note: There should always be at least 1 topic present, since we only 
              construct the Dialogue component when we have a topic to add, and 
              you can't remove topics. */
    std::vector<Topic> topics{};
};

template<typename S>
void serialize(S& serializer, Dialogue::Choice& choice)
{
    serializer.text1b(choice.conditionScript,
                      Dialogue::MAX_CHOICE_CONDITION_SCRIPT_LENGTH);
    serializer.text1b(choice.displayText,
                      Dialogue::MAX_CHOICE_DISPLAY_TEXT_LENGTH);
    serializer.text1b(choice.actionScript,
                      Dialogue::MAX_CHOICE_ACTION_SCRIPT_LENGTH);
}

template<typename S>
void serialize(S& serializer, Dialogue::Topic& topic)
{
    serializer.text1b(topic.topicScript, Dialogue::MAX_TOPIC_SCRIPT_LENGTH);
    serializer.container(topic.choices, Dialogue::MAX_CHOICES);
}

template<typename S>
void serialize(S& serializer, Dialogue& dialogue)
{
    serializer.ext(dialogue.topicIndices,
                   bitsery::ext::StdMap{Dialogue::MAX_TOPICS},
                   [](S& serializer, std::string& name, Uint8& index) {
                       serializer.text1b(name, Dialogue::MAX_TOPIC_NAME_LENGTH);
                       serializer.value1b(index);
                   });
    serializer.container(dialogue.topics, Dialogue::MAX_TOPICS);
}

} // namespace Server
} // namespace AM
