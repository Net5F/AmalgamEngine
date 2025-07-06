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
, animationIDPool{IDPool::ReservationStrategy::ReuseLowest, 32}
, errorString{}
, animationAddedSig{}
, animationRemovedSig{}
, animationDisplayNameChangedSig{}
, animationFrameCountChangedSig{}
, animationFpsChangedSig{}
, animationLoopStartFrameChangedSig{}
, animationFrameChangedSig{}
, animationModelBoundsIDChangedSig{}
, animationCustomModelBoundsChangedSig{}
, animationCollisionEnabledChangedSig{}
, animationEntityAlignmentAnchorChangedSig{}
, animationAdded{animationAddedSig}
, animationRemoved{animationRemovedSig}
, animationDisplayNameChanged{animationDisplayNameChangedSig}
, animationFrameCountChanged{animationFrameCountChangedSig}
, animationFpsChanged{animationFpsChangedSig}
, animationLoopStartFrameChanged{animationLoopStartFrameChangedSig}
, animationFrameChanged{animationFrameChangedSig}
, animationModelBoundsIDChanged{animationModelBoundsIDChangedSig}
, animationCustomModelBoundsChanged{animationCustomModelBoundsChangedSig}
, animationCollisionEnabledChanged{animationCollisionEnabledChangedSig}
, animationEntityAlignmentAnchorChanged{
      animationEntityAlignmentAnchorChangedSig}
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
    for (auto& animationPair : animationMap) {
        // Add the display name.
        EditorAnimation& animation{animationPair.second};
        json["animations"][i]["displayName"] = animation.displayName;

        // Add the numeric ID.
        json["animations"][i]["numericID"] = animation.numericID;

        // Add the frame count, fps, and loop start frame.
        json["animations"][i]["frameCount"] = animation.frameCount;
        json["animations"][i]["fps"] = animation.fps;
        json["animations"][i]["loopStartFrame"] = animation.loopStartFrame;

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

        // Add the entity alignment anchor, if present.
        if (animation.entityAlignmentAnchor) {
            const Vector3& alignmentAnchor{
                animation.entityAlignmentAnchor.value()};
            json["animations"][i]["entityAlignmentAnchor"]["x"]
                = alignmentAnchor.x;
            json["animations"][i]["entityAlignmentAnchor"]["y"]
                = alignmentAnchor.y;
            json["animations"][i]["entityAlignmentAnchor"]["z"]
                = alignmentAnchor.z;
        }

        i++;
    }
}

const EditorAnimation&
    AnimationModel::addOrGetAnimation(std::string_view displayName)
{
    // Try to find an animation with the given name.
    auto animationNameIt{animationNameMap.find(displayName)};
    if (animationNameIt == animationNameMap.end()) {
        // Doesn't exist. Add a new animation and return its ID.
        return addAnimation(displayName);
    }

    // Return the existing animation.
    return animationMap.at(animationNameIt->second);
}

void AnimationModel::remAnimation(AnimationID animationID)
{
    // Find the animation in the map.
    auto animationIt{animationMap.find(animationID)};
    if (animationIt == animationMap.end()) {
        LOG_FATAL("Invalid ID while removing animation.");
    }

    // Free the animation's ID.
    animationIDPool.freeID(animationID);

    // Erase the animation.
    animationNameMap.erase(animationIt->second.displayName);
    animationMap.erase(animationIt);

    // Clear this animation from any sets that reference it.
    dataModel.graphicSetModel.removeGraphicIDFromSets(toGraphicID(animationID));
    dataModel.entityGraphicSetModel.removeGraphicIDFromSets(
        toGraphicID(animationID));

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

    // If loopStartFrame is past the end of the new frame count, pull it back 
    // in.
    EditorAnimation& animation{animationPair->second};
    if (animation.loopStartFrame > newFrameCount) {
        animation.loopStartFrame = newFrameCount;
        animationLoopStartFrameChangedSig.publish(animationID,
                                                  animation.loopStartFrame);
    }

    // Set the new frame count and signal the change.
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

void AnimationModel::setAnimationLoopStartFrame(AnimationID animationID,
                                                Uint8 newLoopStartFrame)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to set loop start frame using invalid animation ID.");
    }

    // If the new frame number isn't valid, exit early.
    EditorAnimation& animation{animationPair->second};
    if (newLoopStartFrame > animation.frameCount) {
        return;
    }

    // Set the new loop start frame and signal the change.
    animation.loopStartFrame = newLoopStartFrame;

    animationLoopStartFrameChangedSig.publish(animationID, newLoopStartFrame);
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

void AnimationModel::setAnimationEntityAlignmentAnchor(
    AnimationID animationID,
    const std::optional<Vector3>& newEntityAlignmentAnchor)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL(
            "Tried to set entityAlignmentAnchor using invalid animation ID.");
    }

    // Set the new entityAlignmentAnchor and signal the change.
    EditorAnimation& animation{animationPair->second};
    animation.entityAlignmentAnchor = newEntityAlignmentAnchor;

    animationEntityAlignmentAnchorChangedSig.publish(animationID,
                                                     newEntityAlignmentAnchor);
}

void AnimationModel::addAnimationFrame(AnimationID animationID,
                                       const EditorSprite& newSprite)
{
    auto animationPair{animationMap.find(animationID)};
    if (animationPair == animationMap.end()) {
        LOG_FATAL("Tried to add frame using invalid animation ID.");
    }

    EditorAnimation& animation{animationPair->second};
    std::vector<const EditorSprite*> frames{animation.getExpandedFrameVector()};

    // If the new sprite isn't the same size as the other sprites, don't add it.
    for (const EditorAnimation::Frame& frame : animation.frames) {
        const SDL_Rect& textureExtent{frame.sprite.get().textureExtent};
        if ((newSprite.textureExtent.w != textureExtent.w)
            || (newSprite.textureExtent.h != textureExtent.h)) {
            return;
        }
    }

    // If there are no filled frames in the animation, and there are empty 
    // frames, add the sprite to the first frame.
    // Note: This should only occur with newly created animations.
    int frameNumber{-1};
    if ((animation.frames.size() == 0) && (animation.frameCount > 0)) {
        frames.at(0) = &newSprite;
        frameNumber = 0;
    }
    else {
        // The animation has filled frames. If the last filled frame has an 
        // empty slot after it, add the sprite to that slot.
        for (std::size_t i{frames.size()}; i-- > 0;) {
            if (frames.at(i)) {
                // Found the last filled frame. Is there an empty frame after it?
                if ((i + 1) < frames.size()) {
                    // There's an empty frame after, add the sprite to it.
                    frames.at(i + 1) = &newSprite;
                    frameNumber = static_cast<Uint8>(i + 1);
                }

                break;
            }
        }

        // If the last frame wasn't followed by an empty slot, add the sprite 
        // to a new frame at the end.
        if (frameNumber == -1) {
            frames.emplace_back(&newSprite);
            frameNumber = static_cast<Uint8>(frames.size() - 1);

            // Update the frame count, since we added a frame.
            animation.frameCount = static_cast<Uint8>(frames.size());
            animationFrameCountChangedSig.publish(animationID,
                                                  animation.frameCount);
            // Note: We don't update loop start because the user will likely be
            //       adding all their sprites at once, in which case we want to 
            //       keep loopStartFrame at 0 (to loop all). The downside is, 
            //       if the user sets their loopStart elsewhere (e.g. the end, 
            //       to loop none) and then adds a sprite, they may need to re-
            //       adjust their loop start.
        }
    }

    // Update the animation and signal the change.
    animation.setFromExpandedFrameVector(frames);
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
    std::vector<const EditorSprite*> frames{animation.getExpandedFrameVector()};

    // Swap the frames.
    const EditorSprite* oldSourceSprite{frames.at(sourceFrameNumber)};
    frames.at(sourceFrameNumber) = frames.at(destFrameNumber);
    frames.at(destFrameNumber) = oldSourceSprite;

    // Update the animation and signal the changes.
    animation.setFromExpandedFrameVector(frames);
    animationFrameChangedSig.publish(animationID, sourceFrameNumber,
                                     frames.at(sourceFrameNumber));
    animationFrameChangedSig.publish(animationID, destFrameNumber,
                                     frames.at(destFrameNumber));
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
            break;
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

    // Add the loop start frame.
    animation.loopStartFrame = animationJson.at("loopStartFrame");

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

    // Add entityAlignmentAnchor, if present.
    if (animationJson.contains("entityAlignmentAnchor")) {
        animation.entityAlignmentAnchor
            = {animationJson.at("entityAlignmentAnchor").at("x"),
               animationJson.at("entityAlignmentAnchor").at("y"),
               animationJson.at("entityAlignmentAnchor").at("z")};
    }

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

const EditorAnimation&
    AnimationModel::addAnimation(std::string_view displayName)
{
    AM_ASSERT(animationNameMap.find(displayName) == animationNameMap.end(),
              "Animation name already in use.");

    AnimationID numericID{
        static_cast<AnimationID>(animationIDPool.reserveID())};

    // Add the new animation to the maps.
    animationMap.emplace(numericID,
                         EditorAnimation{numericID, std::string{displayName}});
    animationNameMap.emplace(displayName, numericID);

    // Default to a non-0 bounding box so it's easier to click.
    EditorAnimation& animation{animationMap.at(numericID)};
    static constexpr BoundingBox defaultBox{{0, 0, 0}, {20, 20, 20}};
    animation.customModelBounds = defaultBox;

    // Signal the new animation to the UI.
    animationAddedSig.publish(numericID, animation);

    return animation;
}

} // End namespace ResourceImporter
} // End namespace AM
