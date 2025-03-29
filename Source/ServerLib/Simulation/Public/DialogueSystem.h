#pragma once

#include "DialogueChoiceRequest.h"
#include "Dialogue.h"
#include "QueuedEvents.h"

namespace AM
{
struct DialogueResponse;
struct CastInfo;

namespace Server
{
class World;
class Network;
struct DialogueLua;
struct DialogueChoiceConditionLua;

/**
 * Handles entity dialogue data requests.
 * 
 * Dialogue is started when a player selects the Talk interaction on an entity.
 * We respond to the Talk interaction by sending the entity's first dialogue 
 * topic. From there, the dialogue may automatically run further topics, or 
 * the player may navigate to other topics by selecting an available choice.
 */
class DialogueSystem
{
public:
    DialogueSystem(World& inWorld, Network& inNetwork,
                   DialogueLua& inDialogueLua,
                   DialogueChoiceConditionLua& inDialogueChoiceConditionLua);

    /**
     * Processes Talk interactions and dialogue choice requests, sending 
     * dialogue data if the request is valid.
     *
     * Dialogue may also update sim state, e.g. when giving items to entities.
     */
    void processDialogueInteractions();

private:
    /** The maximum number of setNextTopic() calls that can be chained (e.g. a 
        choice action script has a call, which leads to a topic script with a 
        call, etc).
        We need to set a max, otherwise people could write infinite loops. */
    const std::size_t TOPIC_NAVIGATION_MAX{5};

    /**
     * Processes a Talk interaction, sending appropriate response messages.
     */
    void processTalkInteraction(const CastInfo& castInfo);

    /**
     * Runs the given topic's topicScript.
     * @return If the script contained a valid setNextTopic(), returns the next 
     *         topic. Else, returns nullptr.
     */
    const Dialogue::Topic* runTopic(const Dialogue& dialogue,
                                    const Dialogue::Topic& topic,
                                    NetworkID clientID);

    /**
     * Processes a dialogue choice request, sending appropriate response 
     * messages.
     */
    void processDialogueChoice(
        const DialogueChoiceRequest& choiceRequest);

    /**
     * Runs the given choice's actionScript.
     * @return If the script contained a valid setNextTopic(), returns the next
     *         topic. Else, returns nullptr.
     */
    const Dialogue::Topic* runChoice(const Dialogue& dialogue,
                                     const Dialogue::Choice& choice,
                                     std::string_view choiceTopicName,
                                     Uint8 choiceIndex, NetworkID clientID);

    /**
     * Validates that the given request has valid data and that the choice's 
     * condition is satisfied by the client entity.
     *
     * @param clientEntity The entity associated with the request's netID.
     * @return If valid, returns the the target entity's Dialogue component. 
     *         Else, returns nullptr and sends an appropriate error message.
     */
    const Dialogue*
        validateChoiceRequest(const DialogueChoiceRequest& choiceRequest,
                              entt::entity clientEntity);

    /**
     * Runs the given choice's condition script.
     * @return true if the script ran successfully and evaluated to true, else 
     *         false. If the script evaluated to false and sendAccessErrorMessage
     *         == true, sends an appropriate error message.
     */
    bool runChoiceCondition(const Dialogue::Choice& choice,
                            entt::entity clientEntity,
                            entt::entity targetEntity, NetworkID clientID,
                            bool sendAccessErrorMessage);

    /**
     * Iterates the given choices, checking their conditions against clientEntity
     * and targetEntity and adding them to the given response if they pass.
     */
    void addChoicesToResponse(const std::vector<Dialogue::Choice>& choices,
                              entt::entity clientEntity,
                              entt::entity targetEntity, NetworkID clientID,
                              DialogueResponse& response);

    /** Used for validating requests and fetching dialogue data. */
    World& world;
    /** Used for receiving dialogue requests and sending dialogue to clients. */
    Network& network;
    /** Used to run topic and choice action scripts. */
    DialogueLua& dialogueLua;
    /** Used to run choice condition scripts. */
    DialogueChoiceConditionLua& dialogueChoiceConditionLua;

    /** A scratch buffer used while processing strings. */
    std::string workString;

    EventQueue<DialogueChoiceRequest> dialogueChoiceRequestQueue;
};

} // namespace Server
} // namespace AM
