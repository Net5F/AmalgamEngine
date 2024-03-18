#include "AnimationModel.h"
#include "DataModel.h"
#include "nlohmann/json.hpp"
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{

AnimationModel::AnimationModel(DataModel& inDataModel)
: dataModel{inDataModel}
, animationMap{}
, animationIDPool{MAX_ANIMATIONS}
, errorString{}
, animationAddedSig{}
, animationRemovedSig{}
, animationDisplayNameChangedSig{}
, animationFrameCountChangedSig{}
, animationFpsChangedSig{}
, animationFrameChangedSig{}
, animationModelBoundsIDChangedSig{}
, animationCustomModelBoundsChangedSig{}
, animationCollisionEnabledChangedSig{}
, animationAdded{animationAddedSig}
, animationRemoved{animationRemovedSig}
, animationDisplayNameChanged{animationDisplayNameChangedSig}
, animationFrameCountChanged{animationFrameCountChangedSig}
, animationFpsChanged{animationFpsChangedSig}
, animationFrameChanged{animationFrameChangedSig}
, animationModelBoundsIDChanged{animationModelBoundsIDChangedSig}
, animationCustomModelBoundsChanged{animationCustomModelBoundsChangedSig}
, animationCollisionEnabledChanged{animationCollisionEnabledChangedSig}
{
    // Reserve the null animation ID (the engine provides it in code, 
    // so we don't need it in the json).
    animationIDPool.reserveID();
}

bool AnimationModel::load(const nlohmann::json& json)
{
    try {
        // Add each animation.
        for (auto& animationJson : json.at("animations").items()) {
            if (!parseAnimation(animationJson.value())) {
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

void AnimationModel::save(nlohmann::json& json)
{
    json["animations"] = nlohmann::json::array();

    // Fill the json with each animation in the model.
    int i{0};
    AnimationID animationID{1};
    for (auto& animationPair : animationMap) {
        // Add the display name.
        EditorAnimation& animation{animationPair.second};
        json["animations"][i]["displayName"] = animation.displayName;

        // Derive the string ID from the display name and add it.
        json["animations"][i]["stringID"]
            = DataModel::deriveStringID(animation.displayName);

        // Add the numeric ID.
        json["animations"][i]["numericID"] = animationID++;

        // Add the frame count and fps.
        json["animations"][i]["frameCount"] = animation.frameCount;
        json["animations"][i]["fps"] = animation.fps;

        // If the animation has any filled frames, add them.
        if ((animation.frames.size() > 0)
            && (animation.frames[0].sprite.get().numericID != NULL_SPRITE_ID)) {
            int j{0};
            for (auto& [frameNumber, sprite] : animation.frames) {
                json["animations"][i]["frames"][j]["frameNumber"] = frameNumber;
                json["animations"][i]["frames"][j]["spriteID"]
                    = sprite.get().numericID;

                j++;
            }
        }
        else {
            // No filled frames, add a null sprite.
            json["animations"][i]["frames"][0]["frameNumber"] = 0;
            json["animations"][i]["frames"][0]["spriteID"] = NULL_SPRITE_ID;
        }

        // Add collisionEnabled.
        json["animations"][i]["collisionEnabled"] = animation.collisionEnabled;

        // Add modelBoundsID.
        // Note: The engine doesn't use this, but this editor needs it.
        json["animations"][i]["modelBoundsID"] = animation.modelBoundsID;

        // Add the model-space bounds.
        // Note: This will either be a shared bounding box or a custom one 
        //       depending on modelBoundsID. The engine doesn't care either 
        //       way, it just needs to know what the bounds are. 
        const BoundingBox& animationModelBounds{
            animation.getModelBounds(dataModel.boundingBoxModel)}; 
        json["animations"][i]["modelBounds"]["minX"]
            = animationModelBounds.minX;
        json["animations"][i]["modelBounds"]["maxX"]
            = animationModelBounds.maxX;
        json["animations"][i]["modelBounds"]["minY"]
            = animationModelBounds.minY;
        json["animations"][i]["modelBounds"]["maxY"]
            = animationModelBounds.maxY;
        json["animations"][i]["modelBounds"]["minZ"]
            = animationModelBounds.minZ;
        json["animations"][i]["modelBounds"]["maxZ"]
            = animationModelBounds.maxZ;

        i++;
    }
}

bool AnimationModel::addAnimation()
{
    AnimationID numericID{
        static_cast<AnimationID>(animationIDPool.reserveID())};

    // Generate a unique name.
    int nameCount{0};
    std::string displayName{"NewAnimation"};
    while (!animationNameIsUnique(numericID, displayName)) {
        displayName = "NewAnimation" + std::to_string(nameCount);
        nameCount++;
    }

    // Add the new animation to the map.
    animationMap.emplace(numericID, EditorAnimation{numericID, displayName});

    // Default to a non-0 bounding box so it's easier to click.
    EditorAnimation& animation{animationMap[numericID]};
    static constexpr BoundingBox defaultBox{0, 20, 0, 20, 0, 20};
    animation.customModelBounds = defaultBox;

    // Signal the new animation to the UI.
    animationAddedSig.publish(numericID, animation);

    // Set the new animation as the active library item.
    dataModel.setActiveAnimation(numericID);

    return true;
}

void AnimationModel::remAnimation(AnimationID animationID)
{
    // Find the animation in the map.
    auto animationIt{animationMap.find(animationID)};
    if (animationIt == animationMap.end()) {
        LOG_FATAL("Invalid ID while removing animation.");
    }

    // Erase the animation.
    animationMap.erase(animationIt);

    // Signal that the animation was erased.
    animationRemovedSig.publish(animationID);
}

const EditorAnimation&
    AnimationModel::getAnimation(AnimationID animationID) const
{
    auto animationIt{animationMap.find(animationID)};
    if (animationIt == animationMap.end()) {
        LOG_FATAL("Tried to get animation with invalid ID: %d",
                  animationID);
    }

    return animationIt->second;
}

void AnimationModel::setAnimationDisplayName(
    AnimationID animationID, const std::string& newDisplayName)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to set name using invalid animation ID.");
    }

    // Set the new display name and make it unique.
    // Note: All characters that a user can enter are valid in the display
    //       name string, so we don't need to validate.
    int appendedNum{0};
    std::string uniqueDisplayName{newDisplayName};
    while (!animationNameIsUnique(animationID, uniqueDisplayName)) {
        uniqueDisplayName = newDisplayName + std::to_string(appendedNum);
        appendedNum++;
    }

    EditorAnimation& animation{animationPair->second};
    animation.displayName = uniqueDisplayName;

    // Signal the change.
    animationDisplayNameChangedSig.publish(animationID,
                                             animation.displayName);
}

void AnimationModel::setAnimationFrameCount(AnimationID animationID,
                                            Uint8 newFrameCount)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to set frame count using invalid animation ID.");
    }

    // Set the new fps and signal the change.
    EditorAnimation& animation{animationPair->second};
    animation.frameCount = newFrameCount;

    animationFrameCountChangedSig.publish(animationID, newFrameCount);
}

void AnimationModel::setAnimationFps(AnimationID animationID, Uint8 newFps)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to set frame count using invalid animation ID.");
    }

    // Set the new fps and signal the change.
    EditorAnimation& animation{animationPair->second};
    animation.fps = newFps;

    animationFpsChangedSig.publish(animationID, newFps);
}

void AnimationModel::setAnimationFrame(AnimationID animationID,
                                       Uint8 frameNumber,
                                       const EditorSprite* newSprite)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to set frame using invalid animation ID.");
    }

    // If newSprite is valid, add it to the frame.
    EditorAnimation& animation{animationPair->second};
    if (newSprite) {
        animation.setFrame(frameNumber, *newSprite);
    }
    else {
        // newSprite is nullptr, clear the frame.
        animation.clearFrame(frameNumber);
    }

    // Signal the change.
    animationFrameChangedSig.publish(animationID, frameNumber, newSprite);
}

void AnimationModel::setAnimationModelBoundsID(AnimationID animationID,
                                         BoundingBoxID newModelBoundsID)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to set modelBoundsID using invalid animation ID.");
    }

    // Set the new ID and signal the change.
    EditorAnimation& animation{animationPair->second};
    animation.modelBoundsID = newModelBoundsID;

    animationModelBoundsIDChangedSig.publish(animationID, newModelBoundsID);
}

void AnimationModel::setAnimationCustomModelBounds(AnimationID animationID,
                                       const BoundingBox& newModelBounds)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to set customModelBounds using invalid animation ID.");
    }

    // Set the new model bounds and signal the change.
    EditorAnimation& animation{animationPair->second};
    animation.customModelBounds = newModelBounds;

    animationCustomModelBoundsChangedSig.publish(animationID, newModelBounds);
}

void AnimationModel::setAnimationCollisionEnabled(AnimationID animationID,
                                                  bool newCollisionEnabled)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to set collisionEnabled using invalid animation ID.");
    }

    // Set the new collisionEnabled and signal the change.
    EditorAnimation& animation{animationPair->second};
    animation.collisionEnabled = newCollisionEnabled;

    animationCollisionEnabledChangedSig.publish(animationID,
                                                newCollisionEnabled);
}

void AnimationModel::resetModelState()
{
    animationMap.clear();
    animationIDPool.freeAllIDs();
}

const std::string& AnimationModel::getErrorString()
{
    return errorString;
}

bool AnimationModel::parseAnimation(const nlohmann::json& animationJson)
{
    // Mark the animation's ID as reserved so it doesn't get reused.
    AnimationID animationID{
        animationJson.at("numericID").get<AnimationID>()};
    animationIDPool.markIDAsReserved(animationID);

    animationMap.emplace(animationID, EditorAnimation{animationID});
    EditorAnimation& animation{animationMap[animationID]};

    // If the display name isn't unique, fail.
    std::string displayName{
        animationJson.at("displayName").get<std::string>()};
    if (!animationNameIsUnique(animationID, displayName)) {
        errorString = "Animation display name isn't unique: ";
        errorString += animation.displayName.c_str();
        return false;
    }

    // Add the display name.
    animation.displayName = displayName;

    // Add the frame count and fps.
    animation.frameCount = animationJson.at("frameCount");
    animation.fps = animationJson.at("fps");

    // Add the frames.
    for (auto& [key, frameJson] : animationJson.at("frames").items()) {
        // Note: We skip frames that contain the null sprite.
        SpriteID spriteID{frameJson.at("spriteID")};
        if (spriteID) {
            const EditorSprite& sprite{
                dataModel.spriteModel.getSprite(spriteID)};
            animation.frames.emplace_back(frameJson.at("frameNumber"), sprite);
        }
    }

    // Add collisionEnabled.
    animation.collisionEnabled = animationJson.at("collisionEnabled");

    // Add modelBoundsID.
    animation.modelBoundsID = animationJson.at("modelBoundsID");

    // Note: We always save to modelBounds (even if this animation uses a shared
    //       bounding box) because that's what the engine uses.
    animation.customModelBounds.minX
        = animationJson.at("modelBounds").at("minX");
    animation.customModelBounds.maxX
        = animationJson.at("modelBounds").at("maxX");
    animation.customModelBounds.minY
        = animationJson.at("modelBounds").at("minY");
    animation.customModelBounds.maxY
        = animationJson.at("modelBounds").at("maxY");
    animation.customModelBounds.minZ
        = animationJson.at("modelBounds").at("minZ");
    animation.customModelBounds.maxZ
        = animationJson.at("modelBounds").at("maxZ");

    // Signal the new animation to the UI.
    animationAddedSig.publish(animationID, animation);

    return true;
}

bool AnimationModel::animationNameIsUnique(AnimationID animationID,
                                               const std::string& displayName)
{
    // Dumbly look through all names for a match.
    // Note: Eventually, this should change to a name map that we keep updated.
    bool isUnique{true};
    for (const auto& animationPair : animationMap) {
        IconID idToCheck{animationPair.first};
        const EditorAnimation& animation{animationPair.second};

        if ((idToCheck != animationID)
            && (displayName == animation.displayName)) {
            isUnique = false;
        }
    }

    return isUnique;
}

} // End namespace ResourceImporter
} // End namespace AM
