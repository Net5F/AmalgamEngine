#pragma once

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
        /** The script to run when this topic is entered. */
        std::string topicScript{};

        /** The choices that will be displayed for the player to select.
            Choices are added to this vector based on their order in the choice
            script. */
        std::vector<Choice> choices{};
    };

    /** A map of topic names -> their index in the topics vector. */
    std::unordered_map<std::string, Uint8> topicIndices{};

    /** Our look-up table for dialogue topics.
        Topics are added to this vector based on their order in the entity's 
        init script. */
    std::vector<Topic> topics{};
};

} // namespace Server
} // namespace AM
