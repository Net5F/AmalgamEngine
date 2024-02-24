#include "GraphicSetModel.h"
#include "DataModel.h"
#include "nlohmann/json.hpp"

namespace AM
{
namespace ResourceImporter
{
GraphicSetModel::GraphicSetModel(DataModel& inDataModel)
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
, graphicSetRemovedSig{}
, graphicSetSlotChangedSig{}
, graphicSetDisplayNameChangedSig{}
, floorAdded{floorAddedSig}
, floorCoveringAdded{floorCoveringAddedSig}
, wallAdded{wallAddedSig}
, objectAdded{objectAddedSig}
, graphicSetRemoved{graphicSetRemovedSig}
, graphicSetSlotChanged{graphicSetSlotChangedSig}
, graphicSetDisplayNameChanged{graphicSetDisplayNameChangedSig}
{
    // Reserve the null ID for each graphic set type (the engine provides it in 
    // code, so we don't need it in the json).
    floorIDPool.reserveID();
    floorCoveringIDPool.reserveID();
    wallIDPool.reserveID();
    objectIDPool.reserveID();
}

bool GraphicSetModel::load(const nlohmann::json& json)
{
    try {
        // Add each type of graphic set.
        for (auto& floorJson : json.at("floors").items()) {
            if (!parseFloorGraphicSet(floorJson.value())) {
                resetModelState();
                return false;
            }
        }
        for (auto& floorCoveringJson : json.at("floorCoverings").items()) {
            if (!parseFloorCoveringGraphicSet(floorCoveringJson.value())) {
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
    saveFloors(json);
    saveFloorCoverings(json);
    saveWalls(json);
    saveObjects(json);
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
    std::array<GraphicID, 1> graphicIDs{NULL_GRAPHIC_ID};
    floorMap.emplace(numericID,
                     EditorFloorGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorFloorGraphicSet& graphicSet{floorMap[numericID]};
    floorAddedSig.publish(numericID, graphicSet);

    // Set the new graphic set as the active library item.
    dataModel.setActiveGraphicSet(GraphicSet::Type::Floor, numericID);

    return true;
}

bool GraphicSetModel::addFloorCovering()
{
    FloorCoveringGraphicSetID numericID{
        static_cast<FloorCoveringGraphicSetID>(floorCoveringIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewFloorCovering"};
    while (!graphicSetNameIsUnique<EditorFloorCoveringGraphicSet>(numericID,
                                                                displayName)) {
        displayName = "NewFloorCovering" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new, empty graphic set to the map.
    std::array<GraphicID, Rotation::Direction::Count> graphicIDs{
        NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID,
        NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID};
    floorCoveringMap.emplace(numericID, EditorFloorCoveringGraphicSet{
                                            numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorFloorCoveringGraphicSet& graphicSet{floorCoveringMap[numericID]};
    floorCoveringAddedSig.publish(numericID, graphicSet);

    // Set the new graphic as the active library item.
    dataModel.setActiveGraphicSet(GraphicSet::Type::FloorCovering, numericID);

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
        NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID};
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
        NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID,
        NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID, NULL_GRAPHIC_ID};
    objectMap.emplace(numericID,
                      EditorObjectGraphicSet{numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorObjectGraphicSet& graphicSet{objectMap[numericID]};
    objectAddedSig.publish(numericID, graphicSet);

    // Set the new graphic as the active library item.
    dataModel.setActiveGraphicSet(GraphicSet::Type::Object, numericID);

    return true;
}

void GraphicSetModel::remFloor(FloorGraphicSetID floorID)
{
    // Find the floor in the map.
    auto floorIt{floorMap.find(floorID)};
    if (floorIt == floorMap.end()) {
        LOG_FATAL("Invalid ID while removing floor.");
    }

    // Erase the floor.
    floorMap.erase(floorIt);

    // Signal that the graphic set was erased.
    graphicSetRemovedSig.publish(GraphicSet::Type::Floor, floorID);
}

void GraphicSetModel::remFloorCovering(FloorCoveringGraphicSetID floorCoveringID)
{
    // Find the floor covering in the map.
    auto floorCoveringIt{floorCoveringMap.find(floorCoveringID)};
    if (floorCoveringIt == floorCoveringMap.end()) {
        LOG_FATAL("Invalid ID while removing floor covering.");
    }

    // Erase the floor covering.
    floorCoveringMap.erase(floorCoveringIt);

    // Signal that the graphic set was erased.
    graphicSetRemovedSig.publish(GraphicSet::Type::FloorCovering,
                                floorCoveringID);
}

void GraphicSetModel::remWall(WallGraphicSetID wallID)
{
    // Find the wall in the map.
    auto wallIt{wallMap.find(wallID)};
    if (wallIt == wallMap.end()) {
        LOG_FATAL("Invalid ID while removing wall.");
    }

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

    // Erase the object.
    objectMap.erase(objectIt);

    // Signal that the graphic set was erased.
    graphicSetRemovedSig.publish(GraphicSet::Type::Object, objectID);
}

const EditorFloorGraphicSet& GraphicSetModel::getFloor(FloorGraphicSetID floorID)
{
    auto floorIt{floorMap.find(floorID)};
    if (floorIt == floorMap.end()) {
        LOG_FATAL("Tried to get floor with invalid ID: %d", floorID);
    }

    return floorIt->second;
}

const EditorFloorCoveringGraphicSet&
    GraphicSetModel::getFloorCovering(FloorCoveringGraphicSetID floorCoveringID)
{
    auto floorCoveringIt{floorCoveringMap.find(floorCoveringID)};
    if (floorCoveringIt == floorCoveringMap.end()) {
        LOG_FATAL("Tried to get floorCovering with invalid ID: %d",
                  floorCoveringID);
    }

    return floorCoveringIt->second;
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
        case GraphicSet::Type::Floor: {
            setGraphicSetDisplayName<EditorFloorGraphicSet>(type, graphicSetID,
                                                          newDisplayName);
            break;
        }
        case GraphicSet::Type::FloorCovering: {
            setGraphicSetDisplayName<EditorFloorCoveringGraphicSet>(
                type, graphicSetID, newDisplayName);
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
        case GraphicSet::Type::Floor: {
            setGraphicSetSlot<EditorFloorGraphicSet>(type, graphicSetID, index,
                                                   newGraphicID);
            break;
        }
        case GraphicSet::Type::FloorCovering: {
            setGraphicSetSlot<EditorFloorCoveringGraphicSet>(type, graphicSetID,
                                                           index, newGraphicID);
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
    floorMap.clear();
    floorCoveringMap.clear();
    wallMap.clear();
    objectMap.clear();
    floorIDPool.freeAllIDs();
    floorCoveringIDPool.freeAllIDs();
    wallIDPool.freeAllIDs();
    objectIDPool.freeAllIDs();
}

void GraphicSetModel::removeGraphicIDFromSets(GraphicID graphicID)
{
    // Given one of the graphic set maps, this will replace all instances of
    // oldGraphicID with newGraphicID and emit a graphicSetGraphicIDChanged.
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

    replaceIDInSets(floorMap, GraphicSet::Type::Floor, graphicID);
    replaceIDInSets(floorCoveringMap, GraphicSet::Type::FloorCovering, graphicID);
    replaceIDInSets(wallMap, GraphicSet::Type::Wall, graphicID);
    replaceIDInSets(objectMap, GraphicSet::Type::Object, graphicID);
}

const std::string& GraphicSetModel::getErrorString()
{
    return errorString;
}

bool GraphicSetModel::parseFloorGraphicSet(const nlohmann::json& graphicSetJson)
{
    FloorGraphicSetID numericID{
        static_cast<FloorGraphicSetID>(floorIDPool.reserveID())};
    std::string displayName{graphicSetJson.at("displayName").get<std::string>()};

    // Add the graphic set's graphics.
    // Note: Floors just have 1 graphic, but the json uses an array in case we
    //       want to add variations in the future.
    const nlohmann::json& graphicIDJson{graphicSetJson.at("graphicIDs")};
    std::array<GraphicID, 1> graphicIDs{};
    for (std::size_t i = 0; i < graphicIDs.size(); ++i) {
        graphicIDs[i] = graphicIDJson[i].get<GraphicID>();
        if (!graphicSetNameIsUnique<EditorFloorGraphicSet>(graphicIDs[i],
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

bool GraphicSetModel::parseFloorCoveringGraphicSet(
    const nlohmann::json& graphicSetJson)
{
    FloorCoveringGraphicSetID numericID{
        static_cast<FloorCoveringGraphicSetID>(floorCoveringIDPool.reserveID())};
    std::string displayName{graphicSetJson.at("displayName").get<std::string>()};

    // Add the graphic set's graphics.
    const nlohmann::json& graphicIDJson{graphicSetJson.at("graphicIDs")};
    std::array<GraphicID, Rotation::Direction::Count> graphicIDs{};
    for (std::size_t i = 0; i < graphicIDs.size(); ++i) {
        graphicIDs[i] = graphicIDJson[i].get<GraphicID>();
        if (!graphicSetNameIsUnique<EditorFloorCoveringGraphicSet>(graphicIDs[i],
                                                                 displayName)) {
            errorString = "Floor covering display name isn't unique: ";
            errorString += displayName.c_str();
            return false;
        }
    }

    // Save the graphic set in the appropriate map.
    floorCoveringMap.emplace(numericID, EditorFloorCoveringGraphicSet{
                                            numericID, displayName, graphicIDs});

    // Signal the new graphic set to the UI.
    EditorFloorCoveringGraphicSet& graphicSet{floorCoveringMap[numericID]};
    floorCoveringAddedSig.publish(numericID, graphicSet);

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
        if (!graphicSetNameIsUnique<EditorWallGraphicSet>(graphicIDs[i],
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
        if (!graphicSetNameIsUnique<EditorObjectGraphicSet>(graphicIDs[i],
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
    if constexpr (std::same_as<T, EditorFloorGraphicSet>) {
        return floorMap;
    }
    else if constexpr (std::same_as<T, EditorFloorCoveringGraphicSet>) {
        return floorCoveringMap;
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

void GraphicSetModel::saveFloors(nlohmann::json& json)
{
    json["floors"] = nlohmann::json::array();

    // Fill the json with each floor graphic set in the model.
    int i{0};
    FloorGraphicSetID graphicSetID{1};
    for (auto& graphicSetPair : floorMap) {
        EditorFloorGraphicSet& graphicSet{graphicSetPair.second};
        json["floors"][i]["displayName"] = graphicSet.displayName;
        json["floors"][i]["stringID"]
            = DataModel::deriveStringID(graphicSet.displayName);
        json["floors"][i]["numericID"] = graphicSetID++;
        for (std::size_t j = 0; j < graphicSet.graphicIDs.size(); ++j) {
            json["floors"][i]["graphicIDs"][j] = graphicSet.graphicIDs[j];
        }

        i++;
    }
}

void GraphicSetModel::saveFloorCoverings(nlohmann::json& json)
{
    json["floorCoverings"] = nlohmann::json::array();

    // Fill the json with each floor covering graphic set in the model.
    int i{0};
    FloorCoveringGraphicSetID graphicSetID{1};
    for (auto& graphicSetPair : floorCoveringMap) {
        EditorFloorCoveringGraphicSet& graphicSet{graphicSetPair.second};
        json["floorCoverings"][i]["displayName"] = graphicSet.displayName;
        json["floorCoverings"][i]["stringID"]
            = DataModel::deriveStringID(graphicSet.displayName);
        json["floorCoverings"][i]["numericID"] = graphicSetID++;
        for (std::size_t j = 0; j < graphicSet.graphicIDs.size(); ++j) {
            json["floorCoverings"][i]["graphicIDs"][j] = graphicSet.graphicIDs[j];
        }

        i++;
    }
}

void GraphicSetModel::saveWalls(nlohmann::json& json)
{
    json["walls"] = nlohmann::json::array();

    // Fill the json with each wall graphic set in the model.
    int i{0};
    WallGraphicSetID graphicSetID{1};
    for (auto& graphicSetPair : wallMap) {
        EditorWallGraphicSet& graphicSet{graphicSetPair.second};
        json["walls"][i]["displayName"] = graphicSet.displayName;
        json["walls"][i]["stringID"]
            = DataModel::deriveStringID(graphicSet.displayName);
        json["walls"][i]["numericID"] = graphicSetID++;
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
    ObjectGraphicSetID graphicSetID{1};
    for (auto& graphicSetPair : objectMap) {
        EditorObjectGraphicSet& graphicSet{graphicSetPair.second};
        json["objects"][i]["displayName"] = graphicSet.displayName;
        json["objects"][i]["stringID"]
            = DataModel::deriveStringID(graphicSet.displayName);
        json["objects"][i]["numericID"] = graphicSetID++;
        for (std::size_t j = 0; j < graphicSet.graphicIDs.size(); ++j) {
            json["objects"][i]["graphicIDs"][j] = graphicSet.graphicIDs[j];
        }

        i++;
    }
}

} // End namespace ResourceImporter
} // End namespace AM
