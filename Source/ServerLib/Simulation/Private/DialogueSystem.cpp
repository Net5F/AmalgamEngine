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
    World& inWorld, Network& inNetwork, DialogueLua& inDialogueLua,
    DialogueChoiceConditionLua& inDialogueChoiceConditionLua)
: world{inWorld}
, network{inNetwork}
, dialogueLua{inDialogueLua}
, dialogueChoiceConditionLua{inDialogueChoiceConditionLua}
, workString{}
, dialogueChoiceRequestQueue{inNetwork.getEventDispatcher()}
{
    // Register a callback for entity Talk interactions.
    world.castHelper.setOnEntityInteractionCompleted(
        EntityInteractionType::Talk,
        [this](const CastInfo& castInfo) { /* processTalkInteraction(castInfo); */ });
}

void DialogueSystem::processDialogueInteractions()
{
    // Process any waiting dialogue choice requests.
    DialogueChoiceRequest dialogueChoiceRequest{};
    while (dialogueChoiceRequestQueue.pop(dialogueChoiceRequest)) {
        processDialogueChoice(dialogueChoiceRequest);
    }
}

void DialogueSystem::processTalkInteraction(const CastInfo& castInfo)
{
    const Dialogue* dialogue{
        world.registry.try_get<Dialogue>(castInfo.targetEntity)};
    if (!dialogue) {
        // Note: This can happen if the init script has an addTalkInteraction() 
        //       but doesn't have any topic() declarations.
        network.serializeAndSend(
            castInfo.clientID,
            SystemMessage{"Error: Tried to Talk to entity that has no "
                          "Dialogue component."});
        return;
    }
    AM_ASSERT(dialogue->topics.size() > 0,
              "Dialogue should always have at least 1 topic.");

    // Run the topic script, following any setNextTopic() and pushing dialogue 
    // events into the response.
    DialogueResponse dialogueResponse{castInfo.targetEntity, 0};
    dialogueLua.luaState["user"] = castInfo.casterEntity;
    dialogueLua.luaState["self"] = castInfo.targetEntity;
    dialogueLua.clientID = castInfo.clientID;
    dialogueLua.dialogueEvents = &(dialogueResponse.dialogueEvents);

    std::size_t topicNavigationCount{0};
    const Dialogue::Topic* lastTopic{&(dialogue->topics[0])};
    const Dialogue::Topic* nextTopic{
        runTopic(*dialogue, dialogue->topics[0], castInfo.clientID)};
    while (nextTopic && (topicNavigationCount < TOPIC_NAVIGATION_MAX)) {
        lastTopic = nextTopic;
        nextTopic = runTopic(*dialogue, *nextTopic, castInfo.clientID);
        topicNavigationCount++;
    }

    // Add the last topic's choices to the response.
    addChoicesToResponse(lastTopic->choices, castInfo.casterEntity,
                         castInfo.targetEntity, castInfo.clientID,
                         dialogueResponse);

    // Send the dialogue to the client.
    network.serializeAndSend(castInfo.clientID, dialogueResponse);
}

void DialogueSystem::processDialogueChoice(
    const DialogueChoiceRequest& choiceRequest)
{
    auto clientEntityIt{world.netIDMap.find(choiceRequest.netID)};
    if (clientEntityIt == world.netIDMap.end()) {
        // Client doesn't exist (may have disconnected), do nothing.
        return;
    }

    // Validate the request.
    entt::entity clientEntity{clientEntityIt->second};
    const Dialogue* dialogue{
        validateChoiceRequest(choiceRequest, clientEntity)};
    if (!dialogue) {
        return;
    }

    // Get the choice that the request is asking to run.
    // Note: These indices were already validated in validateChoice().
    const Dialogue::Topic& choiceTopic{
        dialogue->topics[choiceRequest.topicIndex]};
    const Dialogue::Choice& choice{
        choiceTopic.choices[choiceRequest.choiceIndex]};

    // Run the choice's action script, pushing dialogue events into the response.
    DialogueResponse dialogueResponse{choiceRequest.targetEntity, 0};
    dialogueLua.luaState["user"] = clientEntity;
    dialogueLua.luaState["self"] = choiceRequest.targetEntity;
    dialogueLua.clientID = choiceRequest.netID;
    dialogueLua.dialogueEvents = &(dialogueResponse.dialogueEvents);

    const Dialogue::Topic* nextTopic{
        runChoice(*dialogue, choice, choiceTopic.name,
                  choiceRequest.choiceIndex, choiceRequest.netID)};

    // If the choice contained a valid setNextTopic(), run the next topic script, 
    // following any setNextTopic and pushing dialogue events into the response.
    std::size_t topicNavigationCount{1};
    const Dialogue::Topic* lastTopic{nullptr};
    while (nextTopic && (topicNavigationCount < TOPIC_NAVIGATION_MAX)) {
        lastTopic = nextTopic;
        nextTopic = runTopic(*dialogue, *nextTopic, choiceRequest.netID);
        topicNavigationCount++;
    }

    // If any topics were ran, add the last topic's choices to the response.
    if (lastTopic) {
        addChoicesToResponse(lastTopic->choices, clientEntity,
                             choiceRequest.targetEntity, choiceRequest.netID,
                             dialogueResponse);
    }

    // Send the dialogue to the client.
    network.serializeAndSend(choiceRequest.netID, dialogueResponse);
}

const Dialogue::Topic*
    DialogueSystem::runChoice(const Dialogue& dialogue,
                              const Dialogue::Choice& choice,
                              std::string_view choiceTopicName,
                              Uint8 choiceIndex, NetworkID clientID)
{
    // Run the choice's action script, pushing dialogue events into the response.
    dialogueLua.nextTopicName = "";
    auto scriptResult{dialogueLua.luaState.script(choice.actionScript,
                                                  &sol::script_pass_on_error)};

    if (!(scriptResult.valid())) {
        sol::error err = scriptResult;
        workString.clear();
        workString.append("Error in choice action script. Topic: \"");
        workString.append(choiceTopicName);
        workString.append("\", choiceIndex: ");
        workString.append(std::to_string(choiceIndex));
        workString.append(", error: ");
        workString.append(err.what());
        network.serializeAndSend(clientID, SystemMessage{workString});
        return nullptr;
    }

    // If a setNextTopic() call occurred, check if it's valid.
    if (dialogueLua.nextTopicName != "") {
        auto topicIndexIt{
            dialogue.topicIndices.find(dialogueLua.nextTopicName)};
        if (topicIndexIt != dialogue.topicIndices.end()) {
            // setNextTopic() is valid, return the next topic.
            return &(dialogue.topics[topicIndexIt->second]);
        }
        else {
            workString.clear();
            workString.append("Invalid setNextTopic(). Topic name: \"");
            workString.append(dialogueLua.nextTopicName);
            workString.append("\".");
            network.serializeAndSend(clientID, SystemMessage{workString});
        }
    }

    return nullptr;
}

const Dialogue::Topic* DialogueSystem::runTopic(const Dialogue& dialogue,
                                                const Dialogue::Topic& topic,
                                                NetworkID clientID)
{
    // Run the topic script, pushing dialogue events into the response.
    dialogueLua.nextTopicName = "";
    auto scriptResult{dialogueLua.luaState.script(topic.topicScript,
                                                  &sol::script_pass_on_error)};

    if (!(scriptResult.valid())) {
        sol::error err = scriptResult;
        workString.clear();
        workString.append("Error in topic script. Topic: \"");
        workString.append(topic.name);
        workString.append("\", error: ");
        workString.append(err.what());
        network.serializeAndSend(clientID, SystemMessage{workString});
        return nullptr;
    }

    // If a setNextTopic() call occurred, check if it's valid.
    if (dialogueLua.nextTopicName != "") {
        auto topicIndexIt{
            dialogue.topicIndices.find(dialogueLua.nextTopicName)};
        if (topicIndexIt != dialogue.topicIndices.end()) {
            // setNextTopic() is valid, return the next topic.
            return &(dialogue.topics[topicIndexIt->second]);
        }
        else {
            workString.clear();
            workString.append("Invalid setNextTopic(). Topic name: \"");
            workString.append(dialogueLua.nextTopicName);
            workString.append("\".");
            network.serializeAndSend(clientID, SystemMessage{workString});
        }
    }

    return nullptr;
}

const Dialogue* DialogueSystem::validateChoiceRequest(
    const DialogueChoiceRequest& choiceRequest, entt::entity clientEntity)
{
    entt::registry& registry{world.registry};

    // Check that the dialogue is valid.
    const Dialogue* dialogue{
        registry.try_get<Dialogue>(choiceRequest.targetEntity)};
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
        // This can happen if the entity is re-initialized while a client is 
        // talking to it, and some topics or choices are removed.
        network.serializeAndSend(
            choiceRequest.netID,
            SystemMessage{"Invalid dialogue request."});
        return nullptr;
    }

    // Check that the client entity can access the requested choice.
    const Dialogue::Topic& topic{dialogue->topics[choiceRequest.topicIndex]};
    const Dialogue::Choice& choice{topic.choices[choiceRequest.choiceIndex]};
    if (!(choice.conditionScript.empty())
        && !runChoiceCondition(choice, clientEntity, choiceRequest.targetEntity,
                               choiceRequest.netID, true)) {
        return nullptr;
    }

    return dialogue;
}

bool DialogueSystem::runChoiceCondition(const Dialogue::Choice& choice,
                                        entt::entity clientEntity,
                                        entt::entity targetEntity,
                                        NetworkID clientID,
                                        bool sendAccessErrorMessage)
{
    // Append "r=" to the script so the result gets saved to a variable.
    workString.clear();
    workString.append("r=");
    workString.append(choice.conditionScript);

    // Run the condition script.
    dialogueChoiceConditionLua.luaState["user"] = clientEntity;
    dialogueChoiceConditionLua.luaState["self"] = targetEntity;
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
        // We always send this error, since it's a malformed script.
        network.serializeAndSend(
            clientID, SystemMessage{"Error: Choice condition script did not "
                                    "evaluate to bool type."});
        return false;
    }
    else if (!(conditionResult.get<bool>())) {
        // We only send this error when appropriate (when the player somehow 
        // selects a choice that shouldn't have been sent to them).
        if (sendAccessErrorMessage) {
            network.serializeAndSend(
                clientID, SystemMessage{"Error: Player entity does not have "
                                        "access to selected choice."});
        }
        return false;
    }

    return conditionResult.get<bool>();
}

void DialogueSystem::addChoicesToResponse(
    const std::vector<Dialogue::Choice>& choices, entt::entity clientEntity,
    entt::entity targetEntity, NetworkID clientID, DialogueResponse& response)
{
    Uint8 index{0};
    for (const Dialogue::Choice& choice : choices) {
        // If the client entity can access this choice, add it to the response.
        if (choice.conditionScript.empty()
            || runChoiceCondition(choice, clientEntity, targetEntity, clientID,
                                  false)) {
            response.choices.emplace_back(index, choice.displayText);
        }

        index++;
    }
}

} // End namespace Server
} // End namespace AM
