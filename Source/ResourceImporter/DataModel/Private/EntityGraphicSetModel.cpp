#include "EntityGraphicSetModel.h"
#include "DataModel.h"
#include "nlohmann/json.hpp"

namespace AM
{
namespace ResourceImporter
{
EntityGraphicSetModel::EntityGraphicSetModel(DataModel& inDataModel)
: dataModel{inDataModel}
, entityIDPool{SDL_MAX_UINT16}
, errorString{}
, entityAddedSig{}
, entityRemovedSig{}
, entitySlotChangedSig{}
, entityDisplayNameChangedSig{}
, entityAdded{entityAddedSig}
, entityRemoved{entityRemovedSig}
, entitySlotChanged{entitySlotChangedSig}
, entityDisplayNameChanged{entityDisplayNameChangedSig}
{
    // Reserve the null ID (the engine provides it in code, so we don't need it
    // in the json).
    entityIDPool.reserveID();
}

bool EntityGraphicSetModel::load(const nlohmann::json& json)
{
    try {
        for (auto& entityJson : json.at("entities").items()) {
            if (!parseEntityGraphicSet(entityJson.value())) {
                resetModelState();
                return false;
            }
        }
    } catch (nlohmann::json::exception& e) {
        resetModelState();
        errorString = "Parse failure - ";
        errorString += e.what();
        return false;
    }

    return true;
}

void EntityGraphicSetModel::save(nlohmann::json& json)
{
    json["entities"] = nlohmann::json::array();

    // Fill the json with each entity graphic set in the model.
    int i{0};
    EntityGraphicSetID graphicSetID{1};
    for (auto& graphicSetPair : entityMap) {
        EditorEntityGraphicSet& graphicSet{graphicSetPair.second};
        json["entities"][i]["displayName"] = graphicSet.displayName;
        json["entities"][i]["stringID"]
            = DataModel::deriveStringID(graphicSet.displayName);
        json["entities"][i]["numericID"] = graphicSetID++;

        // Note: Types/Values are parallel arrays to save file space. If we 
        //       switch to a binary format, they can be combined into a single 
        //       struct or used as key/value in a map.
        for (auto& [entityGraphicType, graphicID] : graphicSet.graphicIDs) {
            json["entities"][i]["graphicIDTypes"].push_back(entityGraphicType);
            json["entities"][i]["graphicIDValues"].push_back(graphicID);
        }

        i++;
    }
}

bool EntityGraphicSetModel::addEntity()
{
    EntityGraphicSetID numericID{
        static_cast<EntityGraphicSetID>(entityIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewEntity"};
    while (!graphicSetNameIsUnique(numericID, displayName)) {
        displayName = "NewEntity" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty graphic set to the map.
    std::unordered_map<EntityGraphicType, GraphicID> graphicIDs{};
    graphicIDs.emplace(EntityGraphicType::IdleSouth, NULL_GRAPHIC_ID);
    entityMap.emplace(
        numericID, EditorEntityGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorEntityGraphicSet& graphicSet{entityMap[numericID]};
    entityAddedSig.publish(numericID, graphicSet);

    // Set the new graphic set as the active library item.
    dataModel.setActiveGraphicSet(GraphicSet::Type::Entity, numericID);

    return true;
}

void EntityGraphicSetModel::remEntity(EntityGraphicSetID entityID)
{
    // Find the entity in the map.
    auto entityIt{entityMap.find(entityID)};
    if (entityIt == entityMap.end()) {
        LOG_FATAL("Invalid ID while removing entity.");
    }

    // Erase the entity.
    entityMap.erase(entityIt);

    // Signal that the graphic set was erased.
    entityRemovedSig.publish(entityID);
}

const EditorEntityGraphicSet&
    EntityGraphicSetModel::getEntity(EntityGraphicSetID entityID)
{
    auto entityIt{entityMap.find(entityID)};
    if (entityIt == entityMap.end()) {
        LOG_FATAL("Tried to get entity with invalid ID: %d", entityID);
    }

    return entityIt->second;
}

void EntityGraphicSetModel::setEntityDisplayName(
    EntityGraphicSetID graphicSetID, const std::string& newDisplayName)
{
    auto graphicSetPair{entityMap.find(graphicSetID)};
    if (graphicSetPair == entityMap.end()) {
        LOG_FATAL("Tried to set name using invalid graphic set ID.");
    }

    // Set the new display name and make it unique.
    // Note: All characters that a user can enter are valid in the display
    //       name string, so we don't need to validate.
    int appendedNum{0};
    std::string uniqueDisplayName{newDisplayName};
    while (!graphicSetNameIsUnique(graphicSetID, uniqueDisplayName)) {
        uniqueDisplayName = newDisplayName + std::to_string(appendedNum);
        appendedNum++;
    }

    EditorEntityGraphicSet& graphicSet{graphicSetPair->second};
    graphicSet.displayName = uniqueDisplayName;

    // Signal the change.
    entityDisplayNameChangedSig.publish(graphicSetID, graphicSet.displayName);
}

void EntityGraphicSetModel::setEntitySlot(EntityGraphicSetID graphicSetID,
                                          EntityGraphicType graphicType,
                                          GraphicID newGraphicID)
{
    auto graphicSetPair{entityMap.find(graphicSetID)};
    if (graphicSetPair == entityMap.end()) {
        LOG_FATAL("Tried to set slot using invalid graphic set ID.");
    }

    // If newGraphicID is non-null, update or add it to the map.
    EditorEntityGraphicSet& graphicSet{graphicSetPair->second};
    if (newGraphicID) {
        graphicSet.graphicIDs[graphicType] = newGraphicID;
    }
    // newGraphicID is null. Clear the map entry.
    else {
        // Never erase IdleSouth, just set it to the null sprite.
        if (graphicType == EntityGraphicType::IdleSouth) {
            graphicSet.graphicIDs[graphicType] = toGraphicID(NULL_SPRITE_ID);
        }
        else {
            graphicSet.graphicIDs.erase(graphicType);
        }
    }

    // Signal the change.
    entitySlotChangedSig.publish(graphicSetID, graphicType, newGraphicID);
}

void EntityGraphicSetModel::resetModelState()
{
    entityMap.clear();
    entityIDPool.freeAllIDs();
}

void EntityGraphicSetModel::removeGraphicIDFromSets(GraphicID graphicID)
{
    // For each graphic set in the map.
    for (auto& pair : entityMap) {
        EditorEntityGraphicSet& graphicSet{pair.second};

        // For each slot in the set.
        auto& graphicIDs{graphicSet.graphicIDs};
        for (auto it = graphicIDs.begin(); it != graphicIDs.end();) {
            // If graphicID is present, erase it and signal.
            if (it->second == graphicID) {
                entitySlotChangedSig.publish(graphicSet.numericID, it->first,
                                             NULL_GRAPHIC_ID);
                it = graphicIDs.erase(it);
            }
            else {
                it++;
            }
        }
    }
}

const std::string& EntityGraphicSetModel::getErrorString()
{
    return errorString;
}

bool EntityGraphicSetModel::parseEntityGraphicSet(const nlohmann::json& graphicSetJson)
{
    EntityGraphicSetID numericID{
        static_cast<EntityGraphicSetID>(entityIDPool.reserveID())};
    std::string displayName{graphicSetJson.at("displayName").get<std::string>()};

    // Add the graphic set's graphics.
    // Note: Types/Values are parallel arrays to save file space. If we switch 
    //       to a binary format, they can be combined into a single struct or 
    //       used as key/value in a map.
    const auto& graphicIDTypesJson{graphicSetJson.at("graphicIDTypes")};
    const auto& graphicIDValuesJson{graphicSetJson.at("graphicIDValues")};
    std::unordered_map<EntityGraphicType, GraphicID> graphicIDs{};
    for (std::size_t i{0}; i < graphicIDTypesJson.size(); ++i) {
        EntityGraphicType graphicType{
            graphicIDTypesJson.at(i).get<EntityGraphicType>()};
        GraphicID graphicID{graphicIDValuesJson.at(i).get<GraphicID>()};

        graphicIDs.emplace(graphicType, graphicID);

        if (!graphicSetNameIsUnique(numericID, displayName)) {
            errorString = "Entity display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // If IdleSouth isn't present, error (must always be present).
    if (!(graphicIDs.contains(EntityGraphicType::IdleSouth))) {
        errorString = "Entity is missing Idle graphic: ";
        errorString += displayName.c_str();
        return false;
    }

    // Save the graphic set in the appropriate map.
    entityMap.emplace(
        numericID, EditorEntityGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorEntityGraphicSet& graphicSet{entityMap[numericID]};
    entityAddedSig.publish(numericID, graphicSet);

    return true;
}

bool EntityGraphicSetModel::graphicSetNameIsUnique(
    EntityGraphicSetID graphicSetID, const std::string& displayName)
{
    // Dumbly look through all names for a match.
    // TODO: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& graphicSetPair : entityMap) {
        int idToCheck{graphicSetPair.first};
        const EditorEntityGraphicSet& graphicSet{graphicSetPair.second};

        if ((idToCheck != graphicSetID)
            && (displayName == graphicSet.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

} // End namespace ResourceImporter
} // End namespace AM
