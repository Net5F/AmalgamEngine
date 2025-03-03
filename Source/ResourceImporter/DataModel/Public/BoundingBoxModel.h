#pragma once

#include "BoundingBoxID.h"
#include "EditorBoundingBox.h"
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
 * Holds data for bounding boxes.
 *
 * Note: The engine doesn't actually load this data (or anything from the 
 *       "boundingBoxes" section of ResourceData.json). Instead, we copy the 
 *       relevant bounds data directly into the Sprites and Animations that 
 *       use them.
 */
class BoundingBoxModel
{
public:
    BoundingBoxModel(DataModel& inDataModel);

    /**
     * Attempts to load the "boundingBoxes" section of the given json into this 
     * model.
     *
     * @return true if successful. If false, getErrorString() will return more
     *         information.
     */
    bool load(const nlohmann::json& json);

    /**
     * Saves the current state of this data model into the given json's
     * "boundingBoxes" section.
     */
    void save(nlohmann::json& json);

    /**
     * Adds a default bounding box and loads it.
     */
    bool addBoundingBox();

    /**
     * Adds a bounding box with the given name and bounds, or updates an 
     * existing one if the name is already taken.
     */
    BoundingBoxID addOrUpdateBoundingBox(const std::string& displayName,
                                         const BoundingBox& modelBounds);

    /**
     * Removes the bounding box with the given ID from the associated map.
     *
     * Error if the given ID isn't present in the map.
     */
    void remBoundingBox(BoundingBoxID boundingBoxID);

    const EditorBoundingBox& getBoundingBox(BoundingBoxID boundingBoxID) const;

    // Bounding box properties.
    void setBoundingBoxDisplayName(BoundingBoxID boundingBoxID,
                                   const std::string& newDisplayName);
    void setBoundingBoxBounds(BoundingBoxID boundingBoxID,
                              const BoundingBox& newBounds);

    /** Resets the model state, setting it back to default. */
    void resetModelState();

    const std::string& getErrorString();

private:
    /**
     * @param boundsJson  The json to parse. Must be a valid bounding box
     *                    section from ResourceData.json.
     */
    bool parseBoundingBox(const nlohmann::json& boundsJson);

    /**
     * Checks if the given name is unique among all bounding boxes in the model.
     *
     * @param boundingBoxID  The ID of the bounds that might get displayName. 
     *                       If it already is set to displayName, it won't be 
     *                       counted as non-unique.
     * @param displayName  The display name that the bounds will be set to.
     */
    bool boundingBoxNameIsUnique(BoundingBoxID boundingBoxID,
                                 const std::string& displayName);

    /**
     * Adds a bounding box with the given name, signals to the UI that 
     * the box was added, and returns it.
     * Note: This doesn't check that the name is unique. You must ensure that 
     *       yourself.
     */
    EditorBoundingBox& addBoundingBoxInternal(const std::string& displayName,
                                              const BoundingBox& modelBounds);

    DataModel& dataModel;

    /** Maps bounding box IDs -> the bounding boxes that we currently have 
        loaded. */
    std::map<BoundingBoxID, EditorBoundingBox> boundingBoxMap;

    /** Maps bounding box names -> their ID. */
    std::unordered_map<std::string, BoundingBoxID> boundingBoxNameMap;

    /** Used for generating temporary bounding box IDs that are only used
        internally by this editor. */
    IDPool boundingBoxIDPool;

    /** If one of our parsing functions returns false, this holds a string
        describing the error that occurred. */
    std::string errorString;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(BoundingBoxID boundingBoxID,
                    const EditorBoundingBox& boundingBox)>
        boundingBoxAddedSig;
    entt::sigh<void(BoundingBoxID boundingBoxID)> boundingBoxRemovedSig;

    entt::sigh<void(BoundingBoxID boundingBoxID,
                    const std::string& newDisplayName)>
        boundingBoxDisplayNameChangedSig;
    entt::sigh<void(BoundingBoxID boundingBoxID, const BoundingBox& newBounds)>
        boundingBoxBoundsChangedSig;

public:
    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** A bounding box was added to the model. */
    entt::sink<entt::sigh<void(BoundingBoxID boundingBoxID,
                               const EditorBoundingBox& boundingBox)>>
        boundingBoxAdded;

    /** A bounding box was removed from the model. */
    entt::sink<entt::sigh<void(BoundingBoxID boundingBoxID)>>
        boundingBoxRemoved;

    /** A bounding box's display name has changed. */
    entt::sink<entt::sigh<void(BoundingBoxID boundingBoxID,
                               const std::string& newDisplayName)>>
        boundingBoxDisplayNameChanged;

    /** A bounding box's bounds have changed. */
    entt::sink<entt::sigh<void(BoundingBoxID boundingBoxID,
                               const BoundingBox& newBounds)>>
        boundingBoxBoundsChanged;
};

} // namespace ResourceImporter
} // namespace AM
