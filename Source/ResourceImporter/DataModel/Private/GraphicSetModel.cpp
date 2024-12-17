#include "GraphicSetModel.h"
#include "DataModel.h"
#include "StringTools.h"
#include "nlohmann/json.hpp"

namespace AM
{
namespace ResourceImporter
{
GraphicSetModel::GraphicSetModel(DataModel& inDataModel)
: dataModel{inDataModel}
, terrainIDPool{IDPool::ReservationStrategy::ReuseLowest, 32}
, floorIDPool{IDPool::ReservationStrategy::ReuseLowest, 32}
, wallIDPool{IDPool::ReservationStrategy::ReuseLowest, 32}
, objectIDPool{IDPool::ReservationStrategy::ReuseLowest, 32}
, errorString{}
, terrainAddedSig{}
, floorAddedSig{}
, wallAddedSig{}
, objectAddedSig{}
, graphicSetRemovedSig{}
, graphicSetSlotChangedSig{}
, graphicSetDisplayNameChangedSig{}
, terrainAdded{terrainAddedSig}
, floorAdded{floorAddedSig}
, wallAdded{wallAddedSig}
, objectAdded{objectAddedSig}
, graphicSetRemoved{graphicSetRemovedSig}
, graphicSetSlotChanged{graphicSetSlotChangedSig}
, graphicSetDisplayNameChanged{graphicSetDisplayNameChangedSig}
{
    // Reserve the null ID for each graphic set type (the engine provides it in 
    // code, so we don't need it in the json).
    terrainIDPool.reserveID();
    floorIDPool.reserveID();
    wallIDPool.reserveID();
    objectIDPool.reserveID();
}

bool GraphicSetModel::load(const nlohmann::json& json)
{
    try {
        // Add each type of graphic set.
        for (auto& terrainJson : json.at("terrain").items()) {
            if (!parseTerrainGraphicSet(terrainJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& floorJson : json.at("floors").items()) {
            if (!parseFloorGraphicSet(floorJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& wallJson : json.at("walls").items()) {
            std::string resultString{};
            if (!parseWallGraphicSet(wallJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& objectJson : json.at("objects").items()) {
            if (!parseObjectGraphicSet(objectJson.value())) {
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

void GraphicSetModel::save(nlohmann::json& json)
{
    saveTerrain(json);
    saveFloors(json);
    saveWalls(json);
    saveObjects(json);
}

bool GraphicSetModel::addTerrain()
{
    TerrainGraphicSetID numericID{
        static_cast<TerrainGraphicSetID>(terrainIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewTerrain"};
    while (!graphicSetNameIsUnique<EditorTerrainGraphicSet>(numericID,
                                                                displayName)) {
        displayName = "NewTerrain" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty graphic set to the map.
    std::array<GraphicID, Terrain::Height::Count> graphicIDs{
        /* NULL_GRAPHIC_ID */};
    terrainMap.emplace(numericID, EditorTerrainGraphicSet{
                                            numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorTerrainGraphicSet& graphicSet{terrainMap[numericID]};
    terrainAddedSig.publish(numericID, graphicSet);

    // Set the new graphic as the active library item.
    dataModel.setActiveGraphicSet(GraphicSet::Type::Terrain, numericID);

    return true;
}

bool GraphicSetModel::addFloor()
{
    Uint16 numericID{static_cast<Uint16>(floorIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewFloor"};
    while (
        !graphicSetNameIsUnique<EditorFloorGraphicSet>(numericID, displayName)) {
        displayName = "NewFloor" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty graphic set to the map.
    std::array<GraphicID, Rotation::Direction::Count> graphicIDs{
        /* NULL_GRAPHIC_ID */};
    floorMap.emplace(numericID,
                     EditorFloorGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorFloorGraphicSet& graphicSet{floorMap[numericID]};
    floorAddedSig.publish(numericID, graphicSet);

    // Set the new graphic set as the active library item.
    dataModel.setActiveGraphicSet(GraphicSet::Type::Floor, numericID);

    return true;
}

bool GraphicSetModel::addWall()
{
    WallGraphicSetID numericID{
        static_cast<WallGraphicSetID>(wallIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewWall"};
    while (
        !graphicSetNameIsUnique<EditorWallGraphicSet>(numericID, displayName)) {
        displayName = "NewWall" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty graphic set to the map.
    std::array<GraphicID, Wall::Type::Count> graphicIDs{
        /* NULL_GRAPHIC_ID */};
    wallMap.emplace(numericID,
                    EditorWallGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorWallGraphicSet& graphicSet{wallMap[numericID]};
    wallAddedSig.publish(numericID, graphicSet);

    // Set the new graphic as the active library item.
    dataModel.setActiveGraphicSet(GraphicSet::Type::Wall, numericID);

    return true;
}

bool GraphicSetModel::addObject()
{
    ObjectGraphicSetID numericID{
        static_cast<ObjectGraphicSetID>(objectIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewObject"};
    while (
        !graphicSetNameIsUnique<EditorObjectGraphicSet>(numericID, displayName)) {
        displayName = "NewObject" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty graphic set to the map.
    std::array<GraphicID, Rotation::Direction::Count> graphicIDs{
        /* NULL_GRAPHIC_ID */};
    objectMap.emplace(numericID,
                      EditorObjectGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorObjectGraphicSet& graphicSet{objectMap[numericID]};
    objectAddedSig.publish(numericID, graphicSet);

    // Set the new graphic as the active library item.
    dataModel.setActiveGraphicSet(GraphicSet::Type::Object, numericID);

    return true;
}

void GraphicSetModel::remTerrain(TerrainGraphicSetID terrainID)
{
    // Find the terrain in the map.
    auto terrainIt{terrainMap.find(terrainID)};
    if (terrainIt == terrainMap.end()) {
        LOG_FATAL("Invalid ID while removing terrain.");
    }

    // Free the terrain's ID.
    terrainIDPool.freeID(terrainID);

    // Erase the terrain.
    terrainMap.erase(terrainIt);

    // Signal that the graphic set was erased.
    graphicSetRemovedSig.publish(GraphicSet::Type::Terrain, terrainID);
}

void GraphicSetModel::remFloor(FloorGraphicSetID floorID)
{
    // Find the floor in the map.
    auto floorIt{floorMap.find(floorID)};
    if (floorIt == floorMap.end()) {
        LOG_FATAL("Invalid ID while removing floor.");
    }

    // Free the floor's ID.
    floorIDPool.freeID(floorID);

    // Erase the floor.
    floorMap.erase(floorIt);

    // Signal that the graphic set was erased.
    graphicSetRemovedSig.publish(GraphicSet::Type::Floor, floorID);
}

void GraphicSetModel::remWall(WallGraphicSetID wallID)
{
    // Find the wall in the map.
    auto wallIt{wallMap.find(wallID)};
    if (wallIt == wallMap.end()) {
        LOG_FATAL("Invalid ID while removing wall.");
    }

    // Free the wall's ID.
    wallIDPool.freeID(wallID);

    // Erase the wall.
    wallMap.erase(wallIt);

    // Signal that the graphic set was erased.
    graphicSetRemovedSig.publish(GraphicSet::Type::Wall, wallID);
}

void GraphicSetModel::remObject(ObjectGraphicSetID objectID)
{
    // Find the floor in the map.
    auto objectIt{objectMap.find(objectID)};
    if (objectIt == objectMap.end()) {
        LOG_FATAL("Invalid ID while removing object.");
    }

    // Free the object's ID.
    objectIDPool.freeID(objectID);

    // Erase the object.
    objectMap.erase(objectIt);

    // Signal that the graphic set was erased.
    graphicSetRemovedSig.publish(GraphicSet::Type::Object, objectID);
}

const EditorTerrainGraphicSet&
    GraphicSetModel::getTerrain(TerrainGraphicSetID terrainID)
{
    auto terrainIt{terrainMap.find(terrainID)};
    if (terrainIt == terrainMap.end()) {
        LOG_FATAL("Tried to get terrain with invalid ID: %d", terrainID);
    }

    return terrainIt->second;
}

const EditorFloorGraphicSet& GraphicSetModel::getFloor(FloorGraphicSetID floorID)
{
    auto floorIt{floorMap.find(floorID)};
    if (floorIt == floorMap.end()) {
        LOG_FATAL("Tried to get floor with invalid ID: %d", floorID);
    }

    return floorIt->second;
}

const EditorWallGraphicSet& GraphicSetModel::getWall(WallGraphicSetID wallID)
{
    auto wallIt{wallMap.find(wallID)};
    if (wallIt == wallMap.end()) {
        LOG_FATAL("Tried to get wall with invalid ID: %d", wallID);
    }

    return wallIt->second;
}

const EditorObjectGraphicSet&
    GraphicSetModel::getObject(ObjectGraphicSetID objectID)
{
    auto objectIt{objectMap.find(objectID)};
    if (objectIt == objectMap.end()) {
        LOG_FATAL("Tried to get object with invalid ID: %d", objectID);
    }

    return objectIt->second;
}

void GraphicSetModel::setGraphicSetDisplayName(GraphicSet::Type type,
                                             Uint16 graphicSetID,
                                             const std::string& newDisplayName)
{
    switch (type) {
        case GraphicSet::Type::Terrain: {
            setGraphicSetDisplayName<EditorTerrainGraphicSet>(
                type, graphicSetID, newDisplayName);
            break;
        }
        case GraphicSet::Type::Floor: {
            setGraphicSetDisplayName<EditorFloorGraphicSet>(type, graphicSetID,
                                                          newDisplayName);
            break;
        }
        case GraphicSet::Type::Wall: {
            setGraphicSetDisplayName<EditorWallGraphicSet>(type, graphicSetID,
                                                         newDisplayName);
            break;
        }
        case GraphicSet::Type::Object: {
            setGraphicSetDisplayName<EditorObjectGraphicSet>(type, graphicSetID,
                                                           newDisplayName);
            break;
        }
        default: {
            LOG_FATAL("Invalid graphic set type.");
            break;
        }
    }
}

void GraphicSetModel::setGraphicSetSlot(GraphicSet::Type type, Uint16 graphicSetID,
                                      std::size_t index, GraphicID newGraphicID)
{
    switch (type) {
        case GraphicSet::Type::Terrain: {
            setGraphicSetSlot<EditorTerrainGraphicSet>(type, graphicSetID,
                                                       index, newGraphicID);
            break;
        }
        case GraphicSet::Type::Floor: {
            setGraphicSetSlot<EditorFloorGraphicSet>(type, graphicSetID, index,
                                                   newGraphicID);
            break;
        }
        case GraphicSet::Type::Wall: {
            setGraphicSetSlot<EditorWallGraphicSet>(type, graphicSetID, index,
                                                  newGraphicID);
            break;
        }
        case GraphicSet::Type::Object: {
            setGraphicSetSlot<EditorObjectGraphicSet>(type, graphicSetID, index,
                                                    newGraphicID);
            break;
        }
        default: {
            LOG_FATAL("Invalid graphic set type.");
            break;
        }
    }
}

void GraphicSetModel::resetModelState()
{
    terrainMap.clear();
    floorMap.clear();
    wallMap.clear();
    objectMap.clear();
    terrainIDPool.freeAllIDs();
    floorIDPool.freeAllIDs();
    wallIDPool.freeAllIDs();
    objectIDPool.freeAllIDs();
}

void GraphicSetModel::removeGraphicIDFromSets(GraphicID graphicID)
{
    // Given one of the graphic set maps, this will replace all instances of
    // graphicID with NULL_GRAPHIC_ID and emit a graphicSetGraphicIDChanged.
    auto replaceIDInSets = [this]<typename T>(std::map<Uint16, T>& graphicSetMap,
                                              GraphicSet::Type type,
                                              GraphicID graphicID) {
        // For each graphic set in the map.
        for (auto& pair : graphicSetMap) {
            T& graphicSet{pair.second};

            // For each slot in the set.
            for (std::size_t i = 0; i < graphicSet.graphicIDs.size(); ++i) {
                // If the ID matches oldGraphicID, replace it and signal.
                GraphicID& slotGraphicID{graphicSet.graphicIDs[i]};
                if (slotGraphicID == graphicID) {
                    slotGraphicID = NULL_GRAPHIC_ID;
                    graphicSetSlotChangedSig.publish(type, graphicSet.numericID,
                                                     static_cast<Uint16>(i),
                                                     NULL_GRAPHIC_ID);
                }
            }
        }
    };

    replaceIDInSets(terrainMap, GraphicSet::Type::Terrain, graphicID);
    replaceIDInSets(floorMap, GraphicSet::Type::Floor, graphicID);
    replaceIDInSets(wallMap, GraphicSet::Type::Wall, graphicID);
    replaceIDInSets(objectMap, GraphicSet::Type::Object, graphicID);
}

const std::string& GraphicSetModel::getErrorString()
{
    return errorString;
}

bool GraphicSetModel::parseTerrainGraphicSet(
    const nlohmann::json& graphicSetJson)
{
    TerrainGraphicSetID numericID{
        static_cast<TerrainGraphicSetID>(terrainIDPool.reserveID())};
    std::string displayName{graphicSetJson.at("displayName").get<std::string>()};

    // Add the graphic set's graphics.
    const nlohmann::json& graphicIDJson{graphicSetJson.at("graphicIDs")};
    std::array<GraphicID, Terrain::Height::Count> graphicIDs{};
    for (std::size_t i = 0; i < graphicIDs.size(); ++i) {
        graphicIDs[i] = graphicIDJson[i].get<GraphicID>();
        if (!graphicSetNameIsUnique<EditorTerrainGraphicSet>(
                numericID, displayName)) {
            errorString = "Terrain display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the graphic set in the appropriate map.
    terrainMap.emplace(
        numericID, EditorTerrainGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorTerrainGraphicSet& graphicSet{terrainMap[numericID]};
    terrainAddedSig.publish(numericID, graphicSet);

    return true;
}

bool GraphicSetModel::parseFloorGraphicSet(const nlohmann::json& graphicSetJson)
{
    FloorGraphicSetID numericID{
        static_cast<FloorGraphicSetID>(floorIDPool.reserveID())};
    std::string displayName{graphicSetJson.at("displayName").get<std::string>()};

    // Add the graphic set's graphics.
    const nlohmann::json& graphicIDJson{graphicSetJson.at("graphicIDs")};
    std::array<GraphicID, Rotation::Direction::Count> graphicIDs{};
    for (std::size_t i = 0; i < graphicIDs.size(); ++i) {
        graphicIDs[i] = graphicIDJson[i].get<GraphicID>();
        if (!graphicSetNameIsUnique<EditorFloorGraphicSet>(numericID,
                                                           displayName)) {
            errorString = "Floor display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the graphic set in the appropriate map.
    floorMap.emplace(numericID,
                     EditorFloorGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorFloorGraphicSet& graphicSet{floorMap[numericID]};
    floorAddedSig.publish(numericID, graphicSet);

    return true;
}

bool GraphicSetModel::parseWallGraphicSet(const nlohmann::json& graphicSetJson)
{
    WallGraphicSetID numericID{
        static_cast<WallGraphicSetID>(wallIDPool.reserveID())};
    std::string displayName{graphicSetJson.at("displayName").get<std::string>()};

    // Add the graphic set's graphics.
    const nlohmann::json& graphicIDJson{graphicSetJson.at("graphicIDs")};
    std::array<GraphicID, Wall::Type::Count> graphicIDs{};
    for (std::size_t i = 0; i < graphicIDs.size(); ++i) {
        graphicIDs[i] = graphicIDJson[i].get<GraphicID>();
        if (!graphicSetNameIsUnique<EditorWallGraphicSet>(numericID,
                                                          displayName)) {
            errorString = "Wall display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the graphic set in the appropriate map.
    wallMap.emplace(numericID,
                    EditorWallGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorWallGraphicSet& graphicSet{wallMap[numericID]};
    wallAddedSig.publish(numericID, graphicSet);

    return true;
}

bool GraphicSetModel::parseObjectGraphicSet(const nlohmann::json& graphicSetJson)
{
    ObjectGraphicSetID numericID{
        static_cast<ObjectGraphicSetID>(objectIDPool.reserveID())};
    std::string displayName{graphicSetJson.at("displayName").get<std::string>()};

    // Add the graphic set's graphics.
    const nlohmann::json& graphicIDJson{graphicSetJson.at("graphicIDs")};
    std::array<GraphicID, Rotation::Direction::Count> graphicIDs{};
    for (std::size_t i = 0; i < graphicIDs.size(); ++i) {
        graphicIDs[i] = graphicIDJson[i].get<GraphicID>();
        if (!graphicSetNameIsUnique<EditorObjectGraphicSet>(numericID,
                                                            displayName)) {
            errorString = "Object display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the graphic set in the appropriate map.
    objectMap.emplace(numericID,
                      EditorObjectGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorObjectGraphicSet& graphicSet{objectMap[numericID]};
    objectAddedSig.publish(numericID, graphicSet);

    return true;
}

template<typename T>
std::map<Uint16, T>& GraphicSetModel::getMapForGraphicSetType()
{
    std::map<Uint16, T>* map{nullptr};
    if constexpr (std::same_as<T, EditorTerrainGraphicSet>) {
        return terrainMap;
    }
    else if constexpr (std::same_as<T, EditorFloorGraphicSet>) {
        return floorMap;
    }
    else if constexpr (std::same_as<T, EditorWallGraphicSet>) {
        return wallMap;
    }
    else if constexpr (std::same_as<T, EditorObjectGraphicSet>) {
        return objectMap;
    }
    else {
        LOG_FATAL("Invalid graphic set type.");
        return floorMap;
    }
}

template<typename T>
bool GraphicSetModel::graphicSetNameIsUnique(Uint16 graphicSetID,
                                           const std::string& displayName)
{
    auto& graphicSetMap{getMapForGraphicSetType<T>()};

    // Dumbly look through all names for a match.
    // TODO: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& graphicSetPair : graphicSetMap) {
        int idToCheck{graphicSetPair.first};
        const T& graphicSet{graphicSetPair.second};

        if ((idToCheck != graphicSetID)
            && (displayName == graphicSet.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

template<typename T>
void GraphicSetModel::setGraphicSetDisplayName(GraphicSet::Type type,
                                             Uint16 graphicSetID,
                                             const std::string& newDisplayName)
{
    auto& graphicSetMap{getMapForGraphicSetType<T>()};

    auto graphicSetPair{graphicSetMap.find(graphicSetID)};
    if (graphicSetPair == graphicSetMap.end()) {
        LOG_FATAL("Tried to set name using invalid graphic set ID.");
    }

    // Set the new display name and make it unique.
    // Note: All characters that a user can enter are valid in the display
    //       name string, so we don't need to validate.
    int appendedNum{0};
    std::string uniqueDisplayName{newDisplayName};
    while (!graphicSetNameIsUnique<T>(graphicSetID, uniqueDisplayName)) {
        uniqueDisplayName = newDisplayName + std::to_string(appendedNum);
        appendedNum++;
    }

    T& graphicSet{graphicSetPair->second};
    graphicSet.displayName = uniqueDisplayName;

    // Signal the change.
    graphicSetDisplayNameChangedSig.publish(type, graphicSetID,
                                           graphicSet.displayName);
}

template<typename T>
void GraphicSetModel::setGraphicSetSlot(GraphicSet::Type type, Uint16 graphicSetID,
                                      std::size_t index, GraphicID newGraphicID)
{
    auto& graphicSetMap{getMapForGraphicSetType<T>()};

    auto graphicSetPair{graphicSetMap.find(graphicSetID)};
    if (graphicSetPair == graphicSetMap.end()) {
        LOG_FATAL("Tried to set slot using invalid graphic set ID.");
    }

    // Check that the specified set can hold the given index.
    T& graphicSet{graphicSetPair->second};
    if (index >= graphicSet.graphicIDs.size()) {
        LOG_FATAL("Tried to set slot using out of bounds index.");
    }

    graphicSet.graphicIDs[index] = newGraphicID;

    // Signal the change.
    graphicSetSlotChangedSig.publish(type, graphicSetID, index, newGraphicID);
}

void GraphicSetModel::saveTerrain(nlohmann::json& json)
{
    json["terrain"] = nlohmann::json::array();

    // Fill the json with each terrain graphic set in the model.
    int i{0};
    for (auto& graphicSetPair : terrainMap) {
        EditorTerrainGraphicSet& graphicSet{graphicSetPair.second};
        json["terrain"][i]["displayName"] = graphicSet.displayName;
        json["terrain"][i]["numericID"] = graphicSet.numericID;
        for (std::size_t j = 0; j < graphicSet.graphicIDs.size(); ++j) {
            json["terrain"][i]["graphicIDs"][j] = graphicSet.graphicIDs[j];
        }

        i++;
    }
}

void GraphicSetModel::saveFloors(nlohmann::json& json)
{
    json["floors"] = nlohmann::json::array();

    // Fill the json with each floor graphic set in the model.
    int i{0};
    for (auto& graphicSetPair : floorMap) {
        EditorFloorGraphicSet& graphicSet{graphicSetPair.second};
        json["floors"][i]["displayName"] = graphicSet.displayName;
        json["floors"][i]["numericID"] = graphicSet.numericID;
        for (std::size_t j = 0; j < graphicSet.graphicIDs.size(); ++j) {
            json["floors"][i]["graphicIDs"][j] = graphicSet.graphicIDs[j];
        }

        i++;
    }
}

void GraphicSetModel::saveWalls(nlohmann::json& json)
{
    json["walls"] = nlohmann::json::array();

    // Fill the json with each wall graphic set in the model.
    int i{0};
    for (auto& graphicSetPair : wallMap) {
        EditorWallGraphicSet& graphicSet{graphicSetPair.second};
        json["walls"][i]["displayName"] = graphicSet.displayName;
        json["walls"][i]["numericID"] = graphicSet.numericID;
        for (std::size_t j = 0; j < graphicSet.graphicIDs.size(); ++j) {
            json["walls"][i]["graphicIDs"][j] = graphicSet.graphicIDs[j];
        }

        i++;
    }
}

void GraphicSetModel::saveObjects(nlohmann::json& json)
{
    json["objects"] = nlohmann::json::array();

    // Fill the json with each object graphic set in the model.
    int i{0};
    for (auto& graphicSetPair : objectMap) {
        EditorObjectGraphicSet& graphicSet{graphicSetPair.second};
        json["objects"][i]["displayName"] = graphicSet.displayName;
        json["objects"][i]["numericID"] = graphicSet.numericID;
        for (std::size_t j = 0; j < graphicSet.graphicIDs.size(); ++j) {
            json["objects"][i]["graphicIDs"][j] = graphicSet.graphicIDs[j];
        }

        i++;
    }
}

} // End namespace ResourceImporter
} // End namespace AM
