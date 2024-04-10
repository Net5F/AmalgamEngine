#pragma once

#include "DialogueChoiceRequest.h"
#include "Dialogue.h"
#include "QueuedEvents.h"

namespace AM
{
struct DialogueResponse;
namespace Server
{
class Simulation;
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
    DialogueSystem(Simulation& inSimulation, Network& inNetwork,
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
    /**
     * Processes a Talk interaction, sending appropriate response messages.
     * Note: We pass the indivual members so we can avoid the big Simulation.h 
     *       include.
     */
    void processTalkInteraction(entt::entity clientEntity,
                                entt::entity targetEntity, NetworkID clientID);

    /**
     * Processes a dialogue choice request, sending appropriate response 
     * messages.
     */
    void processDialogueChoice(
        const DialogueChoiceRequest& choiceRequest);

    /**
     * Validates that the given request has valid data and that the choice's 
     * condition is satisfied by the client entity.
     *
     * @param clientEntity The entity associated with the request's netID.
     * @return If valid, returns the the target entity's Dialogue component. 
     *         Else, returns nullptr and sends an appropriate error message.
     */
    const Dialogue*
        validateDialogueChoice(const DialogueChoiceRequest& choiceRequest,
                               entt::entity clientEntity);

    /**
     * Runs the given choice's condition script.
     * @return true if the script ran successfully and evaluated to true, else 
     *         false. If false, sends an appropriate error message.
     */
    bool runChoiceCondition(const Dialogue::Topic& topic,
                            const Dialogue::Choice& choice,
                            entt::entity clientEntity,
                            entt::entity targetEntity, NetworkID clientID);

    /** Used for getting Talk interaction requests. */
    Simulation& simulation;
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
