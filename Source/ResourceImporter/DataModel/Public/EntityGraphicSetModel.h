#pragma once

#include "GraphicSets.h"
#include "EditorEntityGraphicSet.h"
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
 * Holds data for entity graphic sets.
 */
class EntityGraphicSetModel
{
public:
    EntityGraphicSetModel(DataModel& inDataModel);

    /**
     * Attempts to load the "entities" section of the given json into this model.
     *
     * @return true if successful. If false, getErrorString() will return more
     *         information.
     */
    bool load(const nlohmann::json& json);

    /**
     * Saves the current state of this data model into the given json's
     * "entites" section.
     */
    void save(nlohmann::json& json);

    /**
     * Adds a blank entity graphic set and loads it.
     */
    bool addEntity();

    /**
     * Removes the entity graphic set with the given ID from the associated 
     * map.
     *
     * Error if the given ID isn't present in the map.
     *
     * @param entityID  The editor ID of the graphic set to remove.
     */
    void remEntity(EntityGraphicSetID entityID);

    const EditorEntityGraphicSet& getEntity(EntityGraphicSetID entityID);

    // Entity graphic set properties.
    void setEntityDisplayName(EntityGraphicSetID graphicSetID,
                              const std::string& newDisplayName);
    /** If newGraphicID is null, the slot will be cleared. */
    void setEntitySlot(EntityGraphicSetID graphicSetID,
                       EntityGraphicType graphicType,
                       Rotation::Direction direction, GraphicID newGraphicID);

    /** Resets the model state, setting it back to default. */
    void resetModelState();

    /**
     * Iterates all the graphic sets and replaces any instances of graphicID
     * with NULL_GRAPHIC_ID.
     */
    void removeGraphicIDFromSets(GraphicID graphicID);

    const std::string& getErrorString();

private:
    // Parsing functions.
    /**
     * @param graphicSetJson  The json to parse. Must be a valid entity graphic
     *                        set section from ResourceData.json.
     */
    bool parseEntityGraphicSet(const nlohmann::json& graphicSetJson);

    /**
     * Checks if the given name is unique among all entity graphic sets in the 
     * model.
     *
     * @param graphicSetID  The ID of the graphic set that might get displayName.
     *                      If it already is set to displayName, it won't be 
     *                      counted as non-unique.
     * @param displayName  The display name that the graphic set will be set to.
     */
    bool graphicSetNameIsUnique(EntityGraphicSetID graphicSetID,
                                const std::string& displayName);

    DataModel& dataModel;

    // Note: These all use Uint16 instead of the specific ID type, so we can 
    //       interact with them generically.
    /** Maps entity IDs -> the entity graphic sets that we currently have loaded.
     */
    std::map<EntityGraphicSetID, EditorEntityGraphicSet> entityMap;

    /** Used for generating temporary graphic set IDs that are only used
        internally by this editor. */
    IDPool entityIDPool;

    /** If one of our parsing functions returns false, this holds a string
        describing the error that occurred. */
    std::string errorString;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(EntityGraphicSetID entityID,
                    const EditorEntityGraphicSet& entity)>
        entityAddedSig;

    entt::sigh<void(EntityGraphicSetID graphicSetID)> entityRemovedSig;
    entt::sigh<void(EntityGraphicSetID graphicSetID,
                    EntityGraphicType graphicType, Rotation::Direction,
                    GraphicID newGraphicID)>
        entitySlotChangedSig;
    entt::sigh<void(EntityGraphicSetID graphicSetID,
                    const std::string& newDisplayName)>
        entityDisplayNameChangedSig;

public:
    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** A floor graphic set was added to the model. */
    entt::sink<entt::sigh<void(EntityGraphicSetID entityID,
                               const EditorEntityGraphicSet& entity)>>
        entityAdded;

    /** An entity set was removed from the model. */
    entt::sink<entt::sigh<void(EntityGraphicSetID graphicSetID)>> entityRemoved;

    /** An entity graphic set's graphic of the given type was changed.
        If ID == null, the slot was cleared. */
    entt::sink<entt::sigh<void(
        EntityGraphicSetID graphicSetID, EntityGraphicType graphicType,
        Rotation::Direction direction, GraphicID newGraphicID)>>
        entitySlotChanged;

    /** An entity graphic set's display name has changed. */
    entt::sink<entt::sigh<void(EntityGraphicSetID graphicSetID,
                               const std::string& newDisplayName)>>
        entityDisplayNameChanged;
};

} // namespace ResourceImporter
} // namespace AM
