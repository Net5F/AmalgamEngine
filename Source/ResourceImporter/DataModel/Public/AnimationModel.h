#pragma once

#include "AnimationID.h"
#include "EditorAnimation.h"
#include "IDPool.h"
#include "entt/signal/sigh.hpp"
#include "nlohmann/json_fwd.hpp"
#include <map>
#include <string>

namespace AM
{
namespace ResourceImporter
{

class DataModel;

/**
 * Holds data for animations.
 */
class AnimationModel
{
public:
    AnimationModel(DataModel& inDataModel);

    /**
     * Attempts to load the "animations" section of the given json into this 
     * model.
     *
     * @return true if successful. If false, getErrorString() will return more
     *         information.
     */
    bool load(const nlohmann::json& json);

    /**
     * Saves the current state of this data model into the given json's
     * "animations" section.
     */
    void save(nlohmann::json& json);

    /**
     * Adds a default animation and loads it.
     */
    bool addAnimation();

    /**
     * Removes the animation with the given ID from the associated map.
     *
     * Error if the given ID isn't present in the map.
     */
    void remAnimation(AnimationID animationID);

    const EditorAnimation& getAnimation(AnimationID animationID) const;

    // Animation properties.
    void setAnimationDisplayName(AnimationID animationID,
                                   const std::string& newDisplayName);
    void setAnimationFrameCount(AnimationID animationID, Uint8 newFrameCount);
    void setAnimationFps(AnimationID animationID, Uint8 newFps);
    /** 
     * @param frameNumber The frame number that was updated.
     * @param newSprite The frame's new sprite. Set to nullptr to clear frame.
     */
    void setAnimationFrame(AnimationID animationID, Uint8 frameNumber,
                           const EditorSprite* newSprite);
    void setAnimationModelBoundsID(AnimationID animationID,
                                BoundingBoxID newModelBoundsID);
    void setAnimationCustomModelBounds(AnimationID animationID,
                                    const BoundingBox& newModelBounds);
    void setAnimationCollisionEnabled(AnimationID animationID,
                                      bool newCollisionEnabled);

    /** Resets the model state, setting it back to default. */
    void resetModelState();

    const std::string& getErrorString();

private:
    // Note: This was arbitrarily chosen and can be increased if necessary.
    static constexpr std::size_t MAX_ANIMATIONS{1000};

    /**
     * @param animationJson  The json to parse. Must be a valid animation
     *                       section from ResourceData.json.
     */
    bool parseAnimation(const nlohmann::json& animationJson);

    /**
     * Checks if the given name is unique among all animations in the model.
     *
     * @param animationID  The ID of the animation that might get displayName. 
     *                     If it already is set to displayName, it won't be 
     *                     counted as non-unique.
     * @param displayName  The display name that the animation will be set to.
     */
    bool animationNameIsUnique(AnimationID animationID,
                                 const std::string& displayName);

    DataModel& dataModel;

    /** Maps animation IDs -> the animation that we currently have loaded. */
    std::map<AnimationID, EditorAnimation> animationMap;

    /** Used for generating temporary animation IDs that are only used
        internally by this editor. */
    IDPool animationIDPool;

    /** If one of our parsing functions returns false, this holds a string
        describing the error that occurred. */
    std::string errorString;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(AnimationID animationID,
                    const EditorAnimation& animation)>
        animationAddedSig;
    entt::sigh<void(AnimationID animationID)> animationRemovedSig;

    entt::sigh<void(AnimationID animationID,
                    const std::string& newDisplayName)>
        animationDisplayNameChangedSig;
    entt::sigh<void(AnimationID animationID, Uint8 newFrameCount)>
        animationFrameCountChangedSig;
    entt::sigh<void(AnimationID animationID, Uint8 newFps)>
        animationFpsChangedSig;
    entt::sigh<void(AnimationID animationID, Uint8 newFrameNumber,
                    const EditorSprite* newSprite)>
        animationFrameChangedSig;
    entt::sigh<void(AnimationID animationID, BoundingBoxID newModelBoundsID)>
        animationModelBoundsIDChangedSig;
    entt::sigh<void(AnimationID animationID, const BoundingBox& newCustomModelBounds)>
        animationCustomModelBoundsChangedSig;
    entt::sigh<void(AnimationID animationID, bool newCollisionEnabled)>
        animationCollisionEnabledChangedSig;

public:
    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** An animation was added to the model. */
    entt::sink<entt::sigh<void(AnimationID animationID,
                               const EditorAnimation& animation)>>
        animationAdded;

    /** An animation was removed from the model. */
    entt::sink<entt::sigh<void(AnimationID animationID)>>
        animationRemoved;

    /** An animation's display name has changed. */
    entt::sink<entt::sigh<void(AnimationID animationID,
                               const std::string& newDisplayName)>>
        animationDisplayNameChanged;

    /** An animation's frame count has changed. */
    entt::sink<entt::sigh<void(AnimationID animationID, Uint8 newFrameCount)>>
        animationFrameCountChanged;

    /** An animation's fps has changed. */
    entt::sink<entt::sigh<void(AnimationID animationID, Uint8 newFps)>>
        animationFpsChanged;

    /** An animation frame has changed.
        newSprite == nullptr if frame was cleared. */
    entt::sink<entt::sigh<void(AnimationID animationID, Uint8 newFrameNumber,
                               const EditorSprite* newSprite)>>
        animationFrameChanged;

    /** An animation's bounding box ID has changed. */
    entt::sink<
        entt::sigh<void(AnimationID animationID, BoundingBoxID newModelBoundsID)>>
        animationModelBoundsIDChanged;

    /** A animation's custom bounding box has changed. */
    entt::sink<entt::sigh<void(AnimationID animationID,
                               const BoundingBox& newCustomModelBounds)>>
        animationCustomModelBoundsChanged;

    /** An animation's collisionEnabled has changed. */
    entt::sink<
        entt::sigh<void(AnimationID animationID, bool newCollisionEnabled)>>
        animationCollisionEnabledChanged;
};

} // namespace ResourceImporter
} // namespace AM
