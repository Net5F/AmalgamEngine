#include "BoundingBoxModel.h"
#include "DataModel.h"
#include "nlohmann/json.hpp"
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{

BoundingBoxModel::BoundingBoxModel(DataModel& inDataModel)
: dataModel{inDataModel}
, boundingBoxMap{}
, boundingBoxIDPool{MAX_BOUNDING_BOXES}
, errorString{}
, boundingBoxAddedSig{}
, boundingBoxRemovedSig{}
, boundingBoxDisplayNameChangedSig{}
, boundingBoxBoundsChangedSig{}
, boundingBoxAdded{boundingBoxAddedSig}
, boundingBoxRemoved{boundingBoxRemovedSig}
, boundingBoxDisplayNameChanged{boundingBoxDisplayNameChangedSig}
, boundingBoxBoundsChanged{boundingBoxBoundsChangedSig}
{
    // Reserve the null bounding box ID (the engine provides it in code, 
    // so we don't need it in the json).
    boundingBoxIDPool.reserveID();
}

bool BoundingBoxModel::load(const nlohmann::json& json)
{
    try {
        // Add each bounding box.
        for (auto& boundsJson : json.at("boundingBoxes").items()) {
            if (!parseBoundingBox(boundsJson.value())) {
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

void BoundingBoxModel::save(nlohmann::json& json)
{
    json["boundingBoxes"] = nlohmann::json::array();

    // Fill the json with each bounding box in the model.
    int i{0};
    BoundingBoxID boundingBoxID{1};
    for (auto& boundsPair : boundingBoxMap) {
        // Add the display name.
        EditorBoundingBox& bounds{boundsPair.second};
        json["boundingBoxes"][i]["displayName"] = bounds.displayName;

        // Note: We don't need a string ID because the engine never 
        //       directly refers to these bounding boxes.

        // Add the numeric ID.
        json["boundingBoxes"][i]["numericID"] = boundingBoxID++;

        // Add the model-space bounds.
        json["boundingBoxes"][i]["modelBounds"]["minX"]
            = bounds.modelBounds.minX;
        json["boundingBoxes"][i]["modelBounds"]["maxX"]
            = bounds.modelBounds.maxX;
        json["boundingBoxes"][i]["modelBounds"]["minY"]
            = bounds.modelBounds.minY;
        json["boundingBoxes"][i]["modelBounds"]["maxY"]
            = bounds.modelBounds.maxY;
        json["boundingBoxes"][i]["modelBounds"]["minZ"]
            = bounds.modelBounds.minZ;
        json["boundingBoxes"][i]["modelBounds"]["maxZ"]
            = bounds.modelBounds.maxZ;

        i++;
    }
}

bool BoundingBoxModel::addBoundingBox()
{
    BoundingBoxID numericID{
        static_cast<BoundingBoxID>(boundingBoxIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewBoundingBox"};
    while (!boundingBoxNameIsUnique(numericID, displayName)) {
        displayName = "NewBoundingBox" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new bounding box to the map. Default to a non-0 bounding box so 
    // it's easier to click.
    boundingBoxMap.emplace(
        numericID,
        EditorBoundingBox{numericID, displayName, {0, 20, 0, 20, 0, 20}});

    // Signal the new bounding box to the UI.
    EditorBoundingBox& boundingBox{boundingBoxMap[numericID]};
    boundingBoxAddedSig.publish(numericID, boundingBox);

    // Set the new bounding box as the active library item.
    dataModel.setActiveBoundingBox(numericID);

    return true;
}

void BoundingBoxModel::remBoundingBox(BoundingBoxID boundingBoxID)
{
    // Find the boundingBox in the map.
    auto boundingBoxIt{boundingBoxMap.find(boundingBoxID)};
    if (boundingBoxIt == boundingBoxMap.end()) {
        LOG_FATAL("Invalid ID while removing bounding box.");
    }

    // Erase the bounding box.
    boundingBoxMap.erase(boundingBoxIt);

    // Signal that the bounding box was erased.
    boundingBoxRemovedSig.publish(boundingBoxID);
}

const EditorBoundingBox&
    BoundingBoxModel::getBoundingBox(BoundingBoxID boundingBoxID) const
{
    auto boundingBoxIt{boundingBoxMap.find(boundingBoxID)};
    if (boundingBoxIt == boundingBoxMap.end()) {
        LOG_FATAL("Tried to get bounding box with invalid ID: %d",
                  boundingBoxID);
    }

    return boundingBoxIt->second;
}

void BoundingBoxModel::setBoundingBoxDisplayName(
    BoundingBoxID boundingBoxID, const std::string& newDisplayName)
{
    auto boundingBoxPair{boundingBoxMap.find(boundingBoxID)};
    if (boundingBoxPair == boundingBoxMap.end()) {
        LOG_FATAL("Tried to set name using invalid bounding box ID.");
    }

    // Set the new display name and make it unique.
    // Note: All characters that a user can enter are valid in the display
    //       name string, so we don't need to validate.
    int appendedNum{0};
    std::string uniqueDisplayName{newDisplayName};
    while (!boundingBoxNameIsUnique(boundingBoxID, uniqueDisplayName)) {
        uniqueDisplayName = newDisplayName + std::to_string(appendedNum);
        appendedNum++;
    }

    EditorBoundingBox& boundingBox{boundingBoxPair->second};
    boundingBox.displayName = uniqueDisplayName;

    // Signal the change.
    boundingBoxDisplayNameChangedSig.publish(boundingBoxID,
                                             boundingBox.displayName);
}

void BoundingBoxModel::setBoundingBoxBounds(
    BoundingBoxID boundingBoxID, const BoundingBox& newBounds)
{
    auto boundingBoxPair{boundingBoxMap.find(boundingBoxID)};
    if (boundingBoxPair == boundingBoxMap.end()) {
        LOG_FATAL("Tried to set bounds using invalid bounding box ID.");
    }

    EditorBoundingBox& boundingBox{boundingBoxPair->second};
    boundingBox.modelBounds = newBounds;

    // Signal the change.
    boundingBoxBoundsChangedSig.publish(boundingBoxID, boundingBox.modelBounds);
}

void BoundingBoxModel::resetModelState()
{
    boundingBoxMap.clear();
    boundingBoxIDPool.freeAllIDs();
}

const std::string& BoundingBoxModel::getErrorString()
{
    return errorString;
}

bool BoundingBoxModel::parseBoundingBox(const nlohmann::json& boundsJson)
{
    // Mark the bounding box's ID as reserved so it doesn't get reused.
    BoundingBoxID boundingBoxID{
        boundsJson.at("numericID").get<BoundingBoxID>()};
    boundingBoxIDPool.markIDAsReserved(boundingBoxID);

    boundingBoxMap.emplace(boundingBoxID, EditorBoundingBox{boundingBoxID});
    EditorBoundingBox& boundingBox{boundingBoxMap[boundingBoxID]};

    // If the display name isn't unique, fail.
    std::string displayName{
        boundsJson.at("displayName").get<std::string>()};
    if (!boundingBoxNameIsUnique(boundingBoxID, displayName)) {
        errorString = "Bounding box display name isn't unique: ";
        errorString += boundingBox.displayName.c_str();
        return false;
    }

    // Add the display name.
    boundingBox.displayName = displayName;

    // Add the model-space bounds.
    boundingBox.modelBounds.minX = boundsJson.at("modelBounds").at("minX");
    boundingBox.modelBounds.maxX = boundsJson.at("modelBounds").at("maxX");
    boundingBox.modelBounds.minY = boundsJson.at("modelBounds").at("minY");
    boundingBox.modelBounds.maxY = boundsJson.at("modelBounds").at("maxY");
    boundingBox.modelBounds.minZ = boundsJson.at("modelBounds").at("minZ");
    boundingBox.modelBounds.maxZ = boundsJson.at("modelBounds").at("maxZ");

    // Signal the new bounding box to the UI.
    boundingBoxAddedSig.publish(boundingBoxID, boundingBox);

    return true;
}

bool BoundingBoxModel::boundingBoxNameIsUnique(BoundingBoxID boundingBoxID,
                                               const std::string& displayName)
{
    // Dumbly look through all names for a match.
    // Note: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& boundsPair : boundingBoxMap) {
        IconID idToCheck{boundsPair.first};
        const EditorBoundingBox& boundingBox{boundsPair.second};

        if ((idToCheck != boundingBoxID)
            && (displayName == boundingBox.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

} // End namespace ResourceImporter
} // End namespace AM
