#include "SpriteSetModel.h"
#include "DataModel.h"
#include "nlohmann/json.hpp"

namespace AM
{
namespace ResourceImporter
{
SpriteSetModel::SpriteSetModel(DataModel& inDataModel)
: dataModel{inDataModel}
, floorIDPool{SDL_MAX_UINT16}
, floorCoveringIDPool{SDL_MAX_UINT16}
, wallIDPool{SDL_MAX_UINT16}
, objectIDPool{SDL_MAX_UINT16}
, errorString{}
, floorAddedSig{}
, floorCoveringAddedSig{}
, wallAddedSig{}
, objectAddedSig{}
, spriteSetRemovedSig{}
, spriteSetSlotChangedSig{}
, spriteSetDisplayNameChangedSig{}
, floorAdded{floorAddedSig}
, floorCoveringAdded{floorCoveringAddedSig}
, wallAdded{wallAddedSig}
, objectAdded{objectAddedSig}
, spriteSetRemoved{spriteSetRemovedSig}
, spriteSetSlotChanged{spriteSetSlotChangedSig}
, spriteSetDisplayNameChanged{spriteSetDisplayNameChangedSig}
{
}

bool SpriteSetModel::load(const nlohmann::json& json)
{
    try {
        // Add each type of sprite set.
        for (auto& floorJson : json.at("floors").items()) {
            if (!parseFloorSpriteSet(floorJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& floorCoveringJson : json.at("floorCoverings").items()) {
            if (!parseFloorCoveringSpriteSet(floorCoveringJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& wallJson : json.at("walls").items()) {
            std::string resultString{};
            if (!parseWallSpriteSet(wallJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& objectJson : json.at("objects").items()) {
            if (!parseObjectSpriteSet(objectJson.value())) {
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

void SpriteSetModel::save(nlohmann::json& json)
{
    saveFloors(json);
    saveFloorCoverings(json);
    saveWalls(json);
    saveObjects(json);
}

bool SpriteSetModel::addFloor()
{
    Uint16 numericID{static_cast<Uint16>(floorIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewFloor"};
    while (
        !spriteSetNameIsUnique<EditorFloorSpriteSet>(numericID, displayName)) {
        displayName = "NewFloor" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty sprite set to the map.
    std::array<int, 1> spriteIDs{NULL_SPRITE_ID};
    floorMap.emplace(numericID,
                     EditorFloorSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorFloorSpriteSet& spriteSet{floorMap[numericID]};
    floorAddedSig.publish(numericID, spriteSet);

    // Set the new sprite set as the active library item.
    dataModel.setActiveSpriteSet(SpriteSet::Type::Floor, numericID);

    return true;
}

bool SpriteSetModel::addFloorCovering()
{
    Uint16 numericID{static_cast<Uint16>(floorCoveringIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewFloorCovering"};
    while (!spriteSetNameIsUnique<EditorFloorCoveringSpriteSet>(numericID,
                                                                displayName)) {
        displayName = "NewFloorCovering" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty sprite set to the map.
    std::array<int, Rotation::Direction::Count> spriteIDs{
        NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID,
        NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID};
    floorCoveringMap.emplace(numericID, EditorFloorCoveringSpriteSet{
                                            numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorFloorCoveringSpriteSet& spriteSet{floorCoveringMap[numericID]};
    floorCoveringAddedSig.publish(numericID, spriteSet);

    // Set the new sprite as the active library item.
    dataModel.setActiveSpriteSet(SpriteSet::Type::FloorCovering, numericID);

    return true;
}

bool SpriteSetModel::addWall()
{
    Uint16 numericID{static_cast<Uint16>(wallIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewWall"};
    while (
        !spriteSetNameIsUnique<EditorWallSpriteSet>(numericID, displayName)) {
        displayName = "NewWall" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty sprite set to the map.
    std::array<int, Wall::Type::Count> spriteIDs{
        NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID};
    wallMap.emplace(numericID,
                    EditorWallSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorWallSpriteSet& spriteSet{wallMap[numericID]};
    wallAddedSig.publish(numericID, spriteSet);

    // Set the new sprite as the active library item.
    dataModel.setActiveSpriteSet(SpriteSet::Type::Wall, numericID);

    return true;
}

bool SpriteSetModel::addObject()
{
    Uint16 numericID{static_cast<Uint16>(objectIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewObject"};
    while (
        !spriteSetNameIsUnique<EditorObjectSpriteSet>(numericID, displayName)) {
        displayName = "NewObject" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty sprite set to the map.
    std::array<int, Rotation::Direction::Count> spriteIDs{
        NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID,
        NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID, NULL_SPRITE_ID};
    objectMap.emplace(numericID,
                      EditorObjectSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorObjectSpriteSet& spriteSet{objectMap[numericID]};
    objectAddedSig.publish(numericID, spriteSet);

    // Set the new sprite as the active library item.
    dataModel.setActiveSpriteSet(SpriteSet::Type::Object, numericID);

    return true;
}

void SpriteSetModel::remFloor(Uint16 floorID)
{
    // Find the floor in the map.
    auto floorIt{floorMap.find(floorID)};
    if (floorIt == floorMap.end()) {
        LOG_FATAL("Invalid ID while removing floor.");
    }

    // Erase the floor.
    floorMap.erase(floorIt);

    // Signal that the sprite was erased.
    spriteSetRemovedSig.publish(SpriteSet::Type::Floor, floorID);
}

void SpriteSetModel::remFloorCovering(Uint16 floorCoveringID)
{
    // Find the floor covering in the map.
    auto floorCoveringIt{floorCoveringMap.find(floorCoveringID)};
    if (floorCoveringIt == floorCoveringMap.end()) {
        LOG_FATAL("Invalid ID while removing floor covering.");
    }

    // Erase the floor covering.
    floorCoveringMap.erase(floorCoveringIt);

    // Signal that the sprite was erased.
    spriteSetRemovedSig.publish(SpriteSet::Type::FloorCovering,
                                floorCoveringID);
}

void SpriteSetModel::remWall(Uint16 wallID)
{
    // Find the wall in the map.
    auto wallIt{wallMap.find(wallID)};
    if (wallIt == wallMap.end()) {
        LOG_FATAL("Invalid ID while removing wall.");
    }

    // Erase the sprite.
    wallMap.erase(wallIt);

    // Signal that the sprite was erased.
    spriteSetRemovedSig.publish(SpriteSet::Type::Wall, wallID);
}

void SpriteSetModel::remObject(Uint16 objectID)
{
    // Find the floor in the map.
    auto objectIt{objectMap.find(objectID)};
    if (objectIt == objectMap.end()) {
        LOG_FATAL("Invalid ID while removing object.");
    }

    // Erase the sprite.
    objectMap.erase(objectIt);

    // Signal that the sprite was erased.
    spriteSetRemovedSig.publish(SpriteSet::Type::Object, objectID);
}

const EditorFloorSpriteSet& SpriteSetModel::getFloor(Uint16 floorID)
{
    auto floorIt{floorMap.find(floorID)};
    if (floorIt == floorMap.end()) {
        LOG_FATAL("Tried to get floor with invalid ID: %d", floorID);
    }

    return floorIt->second;
}

const EditorFloorCoveringSpriteSet&
    SpriteSetModel::getFloorCovering(Uint16 floorCoveringID)
{
    auto floorCoveringIt{floorCoveringMap.find(floorCoveringID)};
    if (floorCoveringIt == floorCoveringMap.end()) {
        LOG_FATAL("Tried to get floorCovering with invalid ID: %d",
                  floorCoveringID);
    }

    return floorCoveringIt->second;
}

const EditorWallSpriteSet& SpriteSetModel::getWall(Uint16 wallID)
{
    auto wallIt{wallMap.find(wallID)};
    if (wallIt == wallMap.end()) {
        LOG_FATAL("Tried to get wall with invalid ID: %d", wallID);
    }

    return wallIt->second;
}

const EditorObjectSpriteSet& SpriteSetModel::getObject(Uint16 objectID)
{
    auto objectIt{objectMap.find(objectID)};
    if (objectIt == objectMap.end()) {
        LOG_FATAL("Tried to get object with invalid ID: %d", objectID);
    }

    return objectIt->second;
}

void SpriteSetModel::setSpriteSetDisplayName(SpriteSet::Type type,
                                             Uint16 spriteSetID,
                                             const std::string& newDisplayName)
{
    switch (type) {
        case SpriteSet::Type::Floor: {
            setSpriteSetDisplayName<EditorFloorSpriteSet>(type, spriteSetID,
                                                          newDisplayName);
            break;
        }
        case SpriteSet::Type::FloorCovering: {
            setSpriteSetDisplayName<EditorFloorCoveringSpriteSet>(
                type, spriteSetID, newDisplayName);
            break;
        }
        case SpriteSet::Type::Wall: {
            setSpriteSetDisplayName<EditorWallSpriteSet>(type, spriteSetID,
                                                         newDisplayName);
            break;
        }
        case SpriteSet::Type::Object: {
            setSpriteSetDisplayName<EditorObjectSpriteSet>(type, spriteSetID,
                                                           newDisplayName);
            break;
        }
        default: {
            LOG_FATAL("Invalid sprite set type.");
            break;
        }
    }
}

void SpriteSetModel::setSpriteSetSlot(SpriteSet::Type type, Uint16 spriteSetID,
                                      std::size_t index, int newSpriteID)
{
    switch (type) {
        case SpriteSet::Type::Floor: {
            setSpriteSetSlot<EditorFloorSpriteSet>(type, spriteSetID, index,
                                                   newSpriteID);
            break;
        }
        case SpriteSet::Type::FloorCovering: {
            setSpriteSetSlot<EditorFloorCoveringSpriteSet>(type, spriteSetID,
                                                           index, newSpriteID);
            break;
        }
        case SpriteSet::Type::Wall: {
            setSpriteSetSlot<EditorWallSpriteSet>(type, spriteSetID, index,
                                                  newSpriteID);
            break;
        }
        case SpriteSet::Type::Object: {
            setSpriteSetSlot<EditorObjectSpriteSet>(type, spriteSetID, index,
                                                    newSpriteID);
            break;
        }
        default: {
            LOG_FATAL("Invalid sprite set type.");
            break;
        }
    }
}

void SpriteSetModel::resetModelState()
{
    floorMap.clear();
    floorCoveringMap.clear();
    wallMap.clear();
    objectMap.clear();
    floorIDPool.freeAllIDs();
    floorCoveringIDPool.freeAllIDs();
    wallIDPool.freeAllIDs();
    objectIDPool.freeAllIDs();
}

void SpriteSetModel::removeSpriteIDFromSets(int spriteID)
{
    // Given one of the sprite set maps, this will replace all instances of
    // oldSpriteID with newSpriteID and emit a spriteSetSpriteIDChanged.
    auto replaceIDInSets = [this]<typename T>(std::map<Uint16, T>& spriteSetMap,
                                              SpriteSet::Type type,
                                              int spriteID) {
        // For each sprite set in the map.
        for (auto& pair : spriteSetMap) {
            T& spriteSet{pair.second};

            // For each slot in the set.
            for (std::size_t i = 0; i < spriteSet.spriteIDs.size(); ++i) {
                // If the ID matches oldSpriteID, replace it and signal.
                int& slotSpriteID{spriteSet.spriteIDs[i]};
                if (slotSpriteID == spriteID) {
                    slotSpriteID = NULL_SPRITE_ID;
                    spriteSetSlotChangedSig.publish(type, spriteSet.numericID,
                                                    static_cast<Uint16>(i),
                                                    NULL_SPRITE_ID);
                }
            }
        }
    };

    replaceIDInSets(floorMap, SpriteSet::Type::Floor, spriteID);
    replaceIDInSets(floorCoveringMap, SpriteSet::Type::FloorCovering, spriteID);
    replaceIDInSets(wallMap, SpriteSet::Type::Wall, spriteID);
    replaceIDInSets(objectMap, SpriteSet::Type::Object, spriteID);
}

const std::string& SpriteSetModel::getErrorString()
{
    return errorString;
}

bool SpriteSetModel::parseFloorSpriteSet(const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{static_cast<Uint16>(floorIDPool.reserveID())};
    std::string displayName{spriteSetJson.at("displayName").get<std::string>()};

    // Add the sprite set's sprites.
    // Note: Floors just have 1 sprite, but the json uses an array in case we
    //       want to add variations in the future.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    std::array<int, 1> spriteIDs{};
    for (std::size_t i = 0; i < spriteIDs.size(); ++i) {
        spriteIDs[i] = spriteIDJson[i].get<int>();
        if (!spriteSetNameIsUnique<EditorFloorSpriteSet>(spriteIDs[i],
                                                         displayName)) {
            errorString = "Floor display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the sprite set in the appropriate map.
    floorMap.emplace(numericID,
                     EditorFloorSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorFloorSpriteSet& spriteSet{floorMap[numericID]};
    floorAddedSig.publish(numericID, spriteSet);

    return true;
}

bool SpriteSetModel::parseFloorCoveringSpriteSet(
    const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{static_cast<Uint16>(floorCoveringIDPool.reserveID())};
    std::string displayName{spriteSetJson.at("displayName").get<std::string>()};

    // Add the sprite set's sprites.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    std::array<int, Rotation::Direction::Count> spriteIDs{};
    for (std::size_t i = 0; i < spriteIDs.size(); ++i) {
        spriteIDs[i] = spriteIDJson[i].get<int>();
        if (!spriteSetNameIsUnique<EditorFloorCoveringSpriteSet>(spriteIDs[i],
                                                                 displayName)) {
            errorString = "Floor covering display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the sprite set in the appropriate map.
    floorCoveringMap.emplace(numericID, EditorFloorCoveringSpriteSet{
                                            numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorFloorCoveringSpriteSet& spriteSet{floorCoveringMap[numericID]};
    floorCoveringAddedSig.publish(numericID, spriteSet);

    return true;
}

bool SpriteSetModel::parseWallSpriteSet(const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{static_cast<Uint16>(wallIDPool.reserveID())};
    std::string displayName{spriteSetJson.at("displayName").get<std::string>()};

    // Add the sprite set's sprites.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    std::array<int, Wall::Type::Count> spriteIDs{};
    for (std::size_t i = 0; i < spriteIDs.size(); ++i) {
        spriteIDs[i] = spriteIDJson[i].get<int>();
        if (!spriteSetNameIsUnique<EditorWallSpriteSet>(spriteIDs[i],
                                                        displayName)) {
            errorString = "Wall display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the sprite set in the appropriate map.
    wallMap.emplace(numericID,
                    EditorWallSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorWallSpriteSet& spriteSet{wallMap[numericID]};
    wallAddedSig.publish(numericID, spriteSet);

    return true;
}

bool SpriteSetModel::parseObjectSpriteSet(const nlohmann::json& spriteSetJson)
{
    Uint16 numericID{static_cast<Uint16>(objectIDPool.reserveID())};
    std::string displayName{spriteSetJson.at("displayName").get<std::string>()};

    // Add the sprite set's sprites.
    const nlohmann::json& spriteIDJson{spriteSetJson.at("spriteIDs")};
    std::array<int, Rotation::Direction::Count> spriteIDs{};
    for (std::size_t i = 0; i < spriteIDs.size(); ++i) {
        spriteIDs[i] = spriteIDJson[i].get<int>();
        if (!spriteSetNameIsUnique<EditorObjectSpriteSet>(spriteIDs[i],
                                                          displayName)) {
            errorString = "Object display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the sprite set in the appropriate map.
    objectMap.emplace(numericID,
                      EditorObjectSpriteSet{numericID, displayName, spriteIDs});

    // Signal the new sprite set to the UI.
    EditorObjectSpriteSet& spriteSet{objectMap[numericID]};
    objectAddedSig.publish(numericID, spriteSet);

    return true;
}

template<typename T>
std::map<Uint16, T>& SpriteSetModel::getMapForSpriteSetType()
{
    std::map<Uint16, T>* map{nullptr};
    if constexpr (std::same_as<T, EditorFloorSpriteSet>) {
        return floorMap;
    }
    else if constexpr (std::same_as<T, EditorFloorCoveringSpriteSet>) {
        return floorCoveringMap;
    }
    else if constexpr (std::same_as<T, EditorWallSpriteSet>) {
        return wallMap;
    }
    else if constexpr (std::same_as<T, EditorObjectSpriteSet>) {
        return objectMap;
    }
    else {
        LOG_FATAL("Invalid sprite set type.");
        return floorMap;
    }
}

template<typename T>
bool SpriteSetModel::spriteSetNameIsUnique(Uint16 spriteSetID,
                                           const std::string& displayName)
{
    auto& spriteSetMap{getMapForSpriteSetType<T>()};

    // Dumbly look through all names for a match.
    // Note: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& spriteSetPair : spriteSetMap) {
        int idToCheck{spriteSetPair.first};
        const T& spriteSet{spriteSetPair.second};

        if ((idToCheck != spriteSetID)
            && (displayName == spriteSet.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

template<typename T>
void SpriteSetModel::setSpriteSetDisplayName(SpriteSet::Type type,
                                             Uint16 spriteSetID,
                                             const std::string& newDisplayName)
{
    auto& spriteSetMap{getMapForSpriteSetType<T>()};

    auto spriteSetPair{spriteSetMap.find(spriteSetID)};
    if (spriteSetPair == spriteSetMap.end()) {
        LOG_FATAL("Tried to set name using invalid sprite set ID.");
    }

    // Set the new display name and make it unique.
    // Note: All characters that a user can enter are valid in the display
    //       name string, so we don't need to validate.
    int appendedNum{0};
    std::string uniqueDisplayName{newDisplayName};
    while (!spriteSetNameIsUnique<T>(spriteSetID, uniqueDisplayName)) {
        uniqueDisplayName = newDisplayName + std::to_string(appendedNum);
        appendedNum++;
    }

    T& spriteSet{spriteSetPair->second};
    spriteSet.displayName = uniqueDisplayName;

    // Signal the change.
    spriteSetDisplayNameChangedSig.publish(type, spriteSetID,
                                           spriteSet.displayName);
}

template<typename T>
void SpriteSetModel::setSpriteSetSlot(SpriteSet::Type type, Uint16 spriteSetID,
                                      std::size_t index, int newSpriteID)
{
    auto& spriteSetMap{getMapForSpriteSetType<T>()};

    auto spriteSetPair{spriteSetMap.find(spriteSetID)};
    if (spriteSetPair == spriteSetMap.end()) {
        LOG_FATAL("Tried to set slot using invalid sprite set ID.");
    }

    // Check that the specified set can hold the given index.
    T& spriteSet{spriteSetPair->second};
    if (index >= spriteSet.spriteIDs.size()) {
        LOG_FATAL("Tried to set slot using out of bounds index.");
    }

    spriteSet.spriteIDs[index] = newSpriteID;

    // Signal the change.
    spriteSetSlotChangedSig.publish(type, spriteSetID, index, newSpriteID);
}

void SpriteSetModel::saveFloors(nlohmann::json& json)
{
    json["floors"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int i{0};
    for (auto& spriteSetPair : floorMap) {
        // Add this sheet's relative path.
        EditorFloorSpriteSet& spriteSet{spriteSetPair.second};
        json["floors"][i]["displayName"] = spriteSet.displayName;
        json["floors"][i]["stringID"]
            = DataModel::deriveStringID(spriteSet.displayName);
        json["floors"][i]["numericID"] = i;
        for (std::size_t j = 0; j < spriteSet.spriteIDs.size(); ++j) {
            json["floors"][i]["spriteIDs"][j] = spriteSet.spriteIDs[j];
        }

        i++;
    }
}

void SpriteSetModel::saveFloorCoverings(nlohmann::json& json)
{
    json["floorCoverings"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int i{0};
    for (auto& spriteSetPair : floorCoveringMap) {
        // Add this sheet's relative path.
        EditorFloorCoveringSpriteSet& spriteSet{spriteSetPair.second};
        json["floorCoverings"][i]["displayName"] = spriteSet.displayName;
        json["floorCoverings"][i]["stringID"]
            = DataModel::deriveStringID(spriteSet.displayName);
        json["floorCoverings"][i]["numericID"] = i;
        for (std::size_t j = 0; j < spriteSet.spriteIDs.size(); ++j) {
            json["floorCoverings"][i]["spriteIDs"][j] = spriteSet.spriteIDs[j];
        }

        i++;
    }
}

void SpriteSetModel::saveWalls(nlohmann::json& json)
{
    json["walls"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int i{0};
    for (auto& spriteSetPair : wallMap) {
        // Add this sheet's relative path.
        EditorWallSpriteSet& spriteSet{spriteSetPair.second};
        json["walls"][i]["displayName"] = spriteSet.displayName;
        json["walls"][i]["stringID"]
            = DataModel::deriveStringID(spriteSet.displayName);
        json["walls"][i]["numericID"] = i;
        for (std::size_t j = 0; j < spriteSet.spriteIDs.size(); ++j) {
            json["walls"][i]["spriteIDs"][j] = spriteSet.spriteIDs[j];
        }

        i++;
    }
}

void SpriteSetModel::saveObjects(nlohmann::json& json)
{
    json["objects"] = nlohmann::json::array();

    // Fill the json with our current model data.
    // For each sprite sheet.
    int i{0};
    for (auto& spriteSetPair : objectMap) {
        // Add this sheet's relative path.
        EditorObjectSpriteSet& spriteSet{spriteSetPair.second};
        json["objects"][i]["displayName"] = spriteSet.displayName;
        json["objects"][i]["stringID"]
            = DataModel::deriveStringID(spriteSet.displayName);
        json["objects"][i]["numericID"] = i;
        for (std::size_t j = 0; j < spriteSet.spriteIDs.size(); ++j) {
            json["objects"][i]["spriteIDs"][j] = spriteSet.spriteIDs[j];
        }

        i++;
    }
}

} // End namespace ResourceImporter
} // End namespace AM
