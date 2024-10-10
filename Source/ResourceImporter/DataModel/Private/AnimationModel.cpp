#include "AnimationModel.h"
#include "DataModel.h"
#include "StringTools.h"
#include "nlohmann/json.hpp"
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{

AnimationModel::AnimationModel(DataModel& inDataModel)
: dataModel{inDataModel}
, animationMap{}
, animationNameMap{}
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
        std::string stringID{};
        StringTools::deriveStringID(animation.displayName, stringID);
        json["animations"][i]["stringID"] = stringID;

        // Add the numeric ID.
        json["animations"][i]["numericID"] = animationID++;

        // Add the frame count and fps.
        json["animations"][i]["frameCount"] = animation.frameCount;
        json["animations"][i]["fps"] = animation.fps;

        // If the animation has any filled frames, add them.
        // Note: Frame numbers/IDs are parallel arrays to save file space. If we 
        //       switch to a binary format, they can be combined into a single 
        //       struct or used as key/value in a map.
        if ((animation.frames.size() > 0)
            && (animation.frames[0].sprite.get().numericID != NULL_SPRITE_ID)) {
            int j{0};
            for (auto& [frameNumber, sprite] : animation.frames) {
                json["animations"][i]["frameNumbers"][j] = frameNumber;
                json["animations"][i]["spriteIDs"][j] = sprite.get().numericID;

                j++;
            }
        }
        else {
            // No filled frames, add a null sprite.
            json["animations"][i]["frameNumbers"][0] = 0;
            json["animations"][i]["spriteIDs"][0] = NULL_SPRITE_ID;
        }

        // Add collisionEnabled.
        json["animations"][i]["collisionEnabled"] = animation.collisionEnabled;

        // Add modelBoundsID.
        // Note: The engine doesn't use this, but this editor needs it.
        json["animations"][i]["modelBoundsID"] = animation.modelBoundsID;

        // Add the model-space bounds.
        // Note: This will either be a shared bounding box or a custom one 
        //       depending on modelBoundsID. In either case, we always save to 
        //       modelBounds because that's what the engine uses. 
        const BoundingBox& animationModelBounds{
            animation.getModelBounds(dataModel.boundingBoxModel)}; 
        json["animations"][i]["modelBounds"]["minX"]
            = animationModelBounds.min.x;
        json["animations"][i]["modelBounds"]["maxX"]
            = animationModelBounds.max.x;
        json["animations"][i]["modelBounds"]["minY"]
            = animationModelBounds.min.y;
        json["animations"][i]["modelBounds"]["maxY"]
            = animationModelBounds.max.y;
        json["animations"][i]["modelBounds"]["minZ"]
            = animationModelBounds.min.z;
        json["animations"][i]["modelBounds"]["maxZ"]
            = animationModelBounds.max.z;

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

    // Add the new animation to the maps.
    animationMap.emplace(numericID, EditorAnimation{numericID, displayName});
    animationNameMap.emplace(displayName, numericID);

    // Default to a non-0 bounding box so it's easier to click.
    EditorAnimation& animation{animationMap.at(numericID)};
    static constexpr BoundingBox defaultBox{{0, 0, 0}, {20, 20, 20}};
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
    animationNameMap.erase(animationIt->second.displayName);
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

const EditorAnimation*
    AnimationModel::getAnimation(std::string_view displayName) const
{
    auto animationNameIt{animationNameMap.find(displayName)};
    if (animationNameIt == animationNameMap.end()) {
        return nullptr;
    }

    return &(animationMap.at(animationNameIt->second));
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

void AnimationModel::addAnimationFrame(AnimationID animationID,
                                       const EditorSprite& newSprite)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to add frame using invalid animation ID.");
    }

    // If there's room, add to the first empty frame.
    EditorAnimation& animation{animationPair->second};
    Uint8 frameNumber{0};
    if (animation.frames.size() < animation.frameCount) {
        std::vector<EditorAnimation::Frame>& frames{animation.frames};
        for (auto it{frames.begin()}; it != frames.end(); ++it) {
            if (it->frameNumber != frameNumber) {
                // Found an unused number. Insert our frame.
                frames.insert(it, {frameNumber, newSprite});
                break;
            }
            else {
                frameNumber++;
            }
        }
    }
    else {
        // No room. Increase frameCount and add a frame to the end.
        animation.frameCount++;
        animationFrameCountChangedSig.publish(animationID,
                                              animation.frameCount);

        frameNumber = (animation.frameCount - 1);
        animation.frames.emplace_back(frameNumber, newSprite);
    }

    // Signal the change.
    animationFrameChangedSig.publish(animationID, frameNumber, &newSprite);
}

void AnimationModel::swapAnimationFrames(AnimationID animationID,
                                         Uint8 sourceFrameNumber,
                                         Uint8 destFrameNumber)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to swap frames using invalid animation ID.");
    }

    EditorAnimation& animation{animationPair->second};
    AM_ASSERT(sourceFrameNumber < animation.frameCount,
              "Invalid frame number.");
    AM_ASSERT(destFrameNumber < animation.frameCount,
              "Invalid frame number.");

    // Try to find the requested frames (if they're empty, they won't exist).
    std::vector<EditorAnimation::Frame>& frames{animation.frames};
    auto sourceFrame{frames.end()};
    auto destFrame{frames.end()};
    for (auto it{frames.begin()}; it != frames.end(); ++it) {
        if (it->frameNumber == sourceFrameNumber) {
            sourceFrame = it;
        }
        else if (it->frameNumber == destFrameNumber) {
            destFrame = it;
        }
    }

    // If we found both frames, swap their contents.
    const EditorSprite* newSourceSprite{nullptr};
    const EditorSprite* newDestSprite{nullptr};
    if ((sourceFrame != frames.end()) && (destFrame != frames.end())) {
        auto sourceSprite{sourceFrame->sprite};
        sourceFrame->sprite = destFrame->sprite;
        destFrame->sprite = sourceSprite;
        newSourceSprite = &(sourceFrame->sprite.get());
        newDestSprite = &(destFrame->sprite.get());
    }
    else if (sourceFrame != frames.end()) {
        // Swapping source with empty.
        destFrame->sprite = sourceFrame->sprite;
        frames.erase(sourceFrame);
        newDestSprite = &(destFrame->sprite.get());
    }
    else if (sourceFrame != frames.end()) {
        // Swapping dest with empty.
        sourceFrame->sprite = destFrame->sprite;
        frames.erase(destFrame);
        newSourceSprite = &(sourceFrame->sprite.get());
    }
    else {
        // Swapping two empty frames, do nothing.
        return;
    }

    // Signal the changes.
    animationFrameChangedSig.publish(animationID, sourceFrameNumber,
                                     newSourceSprite);
    animationFrameChangedSig.publish(animationID, destFrameNumber,
                                     newDestSprite);
}

void AnimationModel::clearAnimationFrame(AnimationID animationID,
                                         Uint8 frameNumber)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to clear frame using invalid animation ID.");
    }

    EditorAnimation& animation{animationPair->second};
    AM_ASSERT(frameNumber < animation.frameCount, "Invalid frame number.");

    // If a frame matches the given number, erase it.
    std::vector<EditorAnimation::Frame>& frames{animation.frames};
    bool wasErased{false};
    for (auto it = frames.begin(); it != frames.end(); ++it) {
        if (it->frameNumber == frameNumber) {
            frames.erase(it);
            wasErased = true;
        }
    }

    // Signal the change.
    if (wasErased) {
        animationFrameChangedSig.publish(animationID, frameNumber, nullptr);
    }
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
    animationNameMap.emplace(displayName, animationID);

    // Add the display name.
    animation.displayName = displayName;

    // Add the frame count and fps.
    animation.frameCount = animationJson.at("frameCount");
    animation.fps = animationJson.at("fps");

    // Add the frames.
    // Note: Frame numbers/IDs are parallel arrays to save file space. If we 
    //       switch to a binary format, they can be combined into a single 
    //       struct or used as key/value in a map.
    const auto& frameNumbersJson{animationJson.at("frameNumbers")};
    const auto& spriteIdsJson{animationJson.at("spriteIDs")};
    for (std::size_t i{0}; i < frameNumbersJson.size(); ++i) {
        // Note: We skip frames that contain the null sprite.
        SpriteID spriteID{spriteIdsJson.at(i).get<SpriteID>()};
        if (spriteID) {
            Uint8 frameNumber{frameNumbersJson.at(i).get<Uint8>()};
            const EditorSprite& sprite{
                dataModel.spriteModel.getSprite(spriteID)};
            animation.frames.emplace_back(frameNumber, sprite);
        }
    }

    // Add collisionEnabled.
    animation.collisionEnabled = animationJson.at("collisionEnabled");

    // Add modelBoundsID.
    animation.modelBoundsID = animationJson.at("modelBoundsID");

    // Default our custom bounds to the saved modelBounds, regardless of 
    // whether we use a shared bounding box or not.
    animation.customModelBounds.min.x
        = animationJson.at("modelBounds").at("minX");
    animation.customModelBounds.max.x
        = animationJson.at("modelBounds").at("maxX");
    animation.customModelBounds.min.y
        = animationJson.at("modelBounds").at("minY");
    animation.customModelBounds.max.y
        = animationJson.at("modelBounds").at("maxY");
    animation.customModelBounds.min.z
        = animationJson.at("modelBounds").at("minZ");
    animation.customModelBounds.max.z
        = animationJson.at("modelBounds").at("maxZ");

    // Signal the new animation to the UI.
    animationAddedSig.publish(animationID, animation);

    return true;
}

bool AnimationModel::animationNameIsUnique(AnimationID animationID,
                                               const std::string& displayName)
{
    // If the desired name is already in the map, and the owner of the name 
    // isn't the given ID, the name isn't unique.
    auto it{animationNameMap.find(displayName)};
    if ((it != animationNameMap.end()) && (it->second != animationID)) {
        return false;
    }

    // Name isn't in the map, or it's owned by the same ID.
    return true;
}

} // End namespace ResourceImporter
} // End namespace AM
