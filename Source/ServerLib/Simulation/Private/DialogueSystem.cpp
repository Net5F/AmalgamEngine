#include "DialogueSystem.h"
#include "Simulation.h"
#include "World.h"
#include "Network.h"
#include "DialogueLua.h"
#include "DialogueChoiceConditionLua.h"
#include "DialogueResponse.h"
#include "SystemMessage.h"
#include "Dialogue.h"
#include "Log.h"
#include "AMAssert.h"

namespace AM
{
namespace Server
{
DialogueSystem::DialogueSystem(
    Simulation& inSimulation, Network& inNetwork, DialogueLua& inDialogueLua,
    DialogueChoiceConditionLua& inDialogueChoiceConditionLua)
: simulation{inSimulation}
, world{inSimulation.getWorld()}
, network{inNetwork}
, dialogueLua{inDialogueLua}
, dialogueChoiceConditionLua{inDialogueChoiceConditionLua}
, workString{}
, dialogueChoiceRequestQueue{inNetwork.getEventDispatcher()}
{
}

void DialogueSystem::processDialogueInteractions()
{
    // Process any waiting Talk interactions.
    Simulation::EntityInteractionData talkRequest{};
    while (simulation.popEntityInteractionRequest(EntityInteractionType::Talk,
                                                  talkRequest)) {
        processTalkInteraction(talkRequest.clientEntity,
                               talkRequest.targetEntity, talkRequest.clientID);
    }

    // Process any waiting dialogue choice requests.
    DialogueChoiceRequest dialogueChoiceRequest{};
    while (dialogueChoiceRequestQueue.pop(dialogueChoiceRequest)) {
        processDialogueChoice(dialogueChoiceRequest);
    }
}

void DialogueSystem::processTalkInteraction(entt::entity clientEntity,
                                            entt::entity targetEntity,
                                            NetworkID clientID)
{
    const Dialogue* dialogue{world.registry.try_get<Dialogue>(targetEntity)};
    if (!dialogue) {
        // This can happen if the init script has an addTalkInteraction() 
        // but doesn't have any topic() declarations.
        network.serializeAndSend(
            clientID,
            SystemMessage{"Error: Tried to Talk to entity that has no "
                          "Dialogue component."});
        return;
    }
    AM_ASSERT(dialogue->topics.size() > 0,
              "Dialogue should always have at least 1 topic.");

    // Run the topic script, pushing dialogue events into a response.
    DialogueResponse response{clientEntity, 0};
    dialogueLua.dialogueEvents = &(response.dialogueEvents);
    auto result{dialogueLua.luaState.script(
        dialogue->topics[0].topicScript, &sol::script_pass_on_error)};

    if (!(result.valid())) {
        sol::error err = result;
        workString.append("Error in topic script. Topic: \"");
        workString.append(dialogue->topics[0].name);
        workString.append("\", error: ");
        workString.append(err.what());
        network.serializeAndSend(clientID, SystemMessage{workString});
        return;
    }

    // Send the dialogue to the client.
    network.serializeAndSend(clientID, response);
}

void DialogueSystem::processDialogueChoice(
    const DialogueChoiceRequest& choiceRequest)
{
    auto clientEntityIt{world.netIdMap.find(choiceRequest.netID)};
    if (clientEntityIt == world.netIdMap.end()) {
        // Client doesn't exist (may have disconnected), do nothing.
        return;
    }

    // Validate the request.
    entt::entity clientEntity{clientEntityIt->second};
    const Dialogue* dialogue{
        validateDialogueChoice(choiceRequest, clientEntity)};
    if (!dialogue) {
        return;
    }

    const Dialogue::Topic& topic{dialogue->topics[choiceRequest.topicIndex]};
    const Dialogue::Choice& choice{topic.choices[choiceRequest.choiceIndex]};

    // The request is valid. Run the choice's action script, pushing dialogue 
    // events into the response.
    DialogueResponse dialogueResponse{choiceRequest.entity, 0};
    dialogueLua.dialogueEvents = &(dialogueResponse.dialogueEvents);
    auto scriptResult{dialogueLua.luaState.script(choice.actionScript,
                                                  &sol::script_pass_on_error)};

    if (!(scriptResult.valid())) {
        sol::error err = scriptResult;
        workString.clear();
        workString.append("Error in choice action script. Topic: \"");
        workString.append(topic.name);
        workString.append("\", choiceIndex: ");
        workString.append(std::to_string(choiceRequest.choiceIndex));
        workString.append(", error: ");
        workString.append(err.what());
        network.serializeAndSend(choiceRequest.netID,
                                 SystemMessage{err.what()});
        return;
    }

    // TODO: This is wrong. We need to track the latest goto and run this 
    //       on the specified topic (if there is one).
    // Run the topic script, pushing dialogue events into the response.
    scriptResult = dialogueLua.luaState.script(topic.topicScript,
                                               &sol::script_pass_on_error);

    if (!(scriptResult.valid())) {
        sol::error err = scriptResult;
        workString.append("Error in topic script. Topic: \"");
        workString.append(topic.name);
        workString.append("\", error: ");
        workString.append(err.what());
        network.serializeAndSend(choiceRequest.netID,
                                 SystemMessage{workString});
        return;
    }

    // Send the dialogue to the client.
    network.serializeAndSend(choiceRequest.netID, dialogueResponse);
}

const Dialogue* DialogueSystem::validateDialogueChoice(
    const DialogueChoiceRequest& choiceRequest, entt::entity clientEntity)
{
    const Dialogue* dialogue{
        world.registry.try_get<Dialogue>(choiceRequest.entity)};
    if (!dialogue) {
        // This can happen if the init script has an addTalkInteraction() 
        // but doesn't have any topic() declarations.
        network.serializeAndSend(
            choiceRequest.netID,
            SystemMessage{"Error: Tried to Talk to entity that has no "
                          "Dialogue component."});
        return nullptr;
    }
    else if ((choiceRequest.topicIndex >= dialogue->topics.size())
             || (choiceRequest.choiceIndex
                 >= dialogue->topics[choiceRequest.topicIndex]
                        .choices.size())) {
        // This can happen if the entity is re-initialized while talking 
        // to it.
        network.serializeAndSend(
            choiceRequest.netID,
            SystemMessage{"Invalid dialogue request."});
        return nullptr;
    }

    // Check if the client entity can access the requested choice.
    const Dialogue::Topic& topic{dialogue->topics[choiceRequest.topicIndex]};
    const Dialogue::Choice& choice{topic.choices[choiceRequest.choiceIndex]};
    if (!runChoiceCondition(topic, choice, clientEntity, choiceRequest.entity,
                            choiceRequest.netID)) {
        return nullptr;
    }

    return dialogue;
}

bool DialogueSystem::runChoiceCondition(const Dialogue::Topic& topic,
                                        const Dialogue::Choice& choice,
                                        entt::entity clientEntity,
                                        entt::entity targetEntity,
                                        NetworkID clientID)
{
    // Append "r=" to the script so the result gets saved to a variable.
    workString.clear();
    workString.append("r=");
    workString.append(choice.conditionScript);

    // Run the condition script.
    dialogueChoiceConditionLua.clientEntity = clientEntity;
    dialogueChoiceConditionLua.targetEntity = targetEntity;
    auto scriptResult{dialogueChoiceConditionLua.luaState.script(
        workString, &sol::script_pass_on_error)};
    if (!(scriptResult.valid())) {
        sol::error err = scriptResult;
        workString.clear();
        workString.append("Choice condition script error: ");
        workString.append(err.what());
        network.serializeAndSend(clientID, SystemMessage{workString});
        return false;
    }

    // Validate the result.
    const auto& conditionResult{dialogueChoiceConditionLua.luaState["r"]};
    if (!(conditionResult.is<bool>())) {
        network.serializeAndSend(
            clientID,
            SystemMessage{
                "Choice condition script did not evaluate to bool type."});
        return false;
    }
    else if (!(conditionResult.get<bool>())) {
        network.serializeAndSend(
            clientID,
            SystemMessage{
                "Client entity does not have access to selected choice."});
        return false;
    }

    return true;
}

} // End namespace Server
} // End namespace AM
