#pragma once

#include "SpriteSets.h"
#include "EditorFloorSpriteSet.h"
#include "EditorFloorCoveringSpriteSet.h"
#include "EditorWallSpriteSet.h"
#include "EditorObjectSpriteSet.h"
#include "IDPool.h"
#include "entt/signal/sigh.hpp"
#include "nlohmann/json_fwd.hpp"
#include <map>
#include <string>

namespace AM
{
namespace SpriteEditor
{

class DataModel;

/**
 */
class SpriteSetModel
{
public:
    SpriteSetModel(DataModel& inDataModel);

    /**
     * Attempts to load the "floors", "floorCoverings", "walls", and "objects" 
     * sections of the given json into this model.
     *
     * @return true if successful. If false, getErrorString() will return more 
     *         information.
     */
    bool load(const nlohmann::json& json);

    /**
     * Saves the current state of this data model into the given json's 
     * "floors", "floorCoverings", "walls", and "objects" sections.
     */
    void save(nlohmann::json& json);

    /**
     * Adds a blank sprite set of the appropriate type and loads it.
     */
    bool addFloor();
    bool addFloorCovering();
    bool addWall();
    bool addObject();

    /**
     * Removes the sprite set of the appropriate type with the given ID from 
     * the associated map.
     * 
     * Error if the given ID isn't preset in the map.
     *
     * @param spriteSetID  The editor ID of the sprite set to remove.
     */
    void remFloor(Uint16 floorID);
    void remFloorCovering(Uint16 floorCoveringID);
    void remWall(Uint16 wallID);
    void remObject(Uint16 objectID);

    const EditorFloorSpriteSet& getFloor(Uint16 floorID);
    const EditorFloorCoveringSpriteSet& getFloorCovering(Uint16 floorCoveringID);
    const EditorWallSpriteSet& getWall(Uint16 wallID);
    const EditorObjectSpriteSet& getObject(Uint16 objectID);

    // Sprite set properties.
    void setSpriteSetDisplayName(SpriteSet::Type type, Uint16 spriteSetID,
                                 const std::string& newDisplayName);
    void setSpriteSetSlot(SpriteSet::Type type, Uint16 spriteSetID,
                          std::size_t index, int newSpriteID);

    /** Resets the model state, setting it back to default. */
    void resetModelState();

    /**
     * Iterates all the sprite sets and replaces any instances of spriteID 
     * with NULL_SPRITE_ID.
     */
    void removeSpriteIDFromSets(int spriteID);

    const std::string& getErrorString();

    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** A floor sprite set was added to the model. */
    entt::sink<entt::sigh<void(Uint16 floorID, const EditorFloorSpriteSet& floor)>>
        floorAdded;
    /** A floor covering sprite set was added to the model. */
    entt::sink<entt::sigh<void(Uint16 floorCoveringID,
                               const EditorFloorCoveringSpriteSet& floorCovering)>>
        floorCoveringAdded;
    /** A wall sprite set was added to the model. */
    entt::sink<
        entt::sigh<void(Uint16 wallID, const EditorWallSpriteSet& wall)>>
        wallAdded;
    /** An object sprite set was added to the model. */
    entt::sink<
        entt::sigh<void(Uint16 objectID, const EditorObjectSpriteSet& floor)>>
        objectAdded;

    /** A sprite set was removed from the model. */
    entt::sink<entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID)>>
        spriteSetRemoved;

    /** A sprite set's sprite at the given index was changed. */
    entt::sink<entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID,
                               std::size_t index, int newSpriteID)>>
        spriteSetSlotChanged;

    /** A sprite set's display name has changed. */
    entt::sink<entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID,
                               const std::string& newDisplayName)>>
        spriteSetDisplayNameChanged;

private:
    // Parsing functions.
    /**
     * @param spriteSetJson  The json to parse. Must be a valid sprite set 
     *                       section from ResourceData.json, for the appropriate 
     *                       set type.
     */
    bool parseFloorSpriteSet(const nlohmann::json& spriteSetJson);
    bool parseFloorCoveringSpriteSet(const nlohmann::json& spriteSetJson);
    bool parseWallSpriteSet(const nlohmann::json& spriteSetJson);
    bool parseObjectSpriteSet(const nlohmann::json& spriteSetJson);

    /**
     * Returns the appropriate map for the given sprite set type.
     */
    template<typename T>
    std::map<Uint16, T>& getMapForSpriteSetType();

    /**
     * Checks if the given name is unique among all sprites in the model.
     *
     * @param spriteID  The ID of the sprite that might get displayName. If it
     *                  already is set to displayName, it won't be counted as
     *                  non-unique.
     * @param displayName  The display name that the sprite will be set to.
     */
    template<typename T>
    bool spriteSetNameIsUnique(Uint16 spriteSetID,
                               const std::string& displayName);

    /**
     * Implementation for setSpriteSetDisplayName().
     */
    template<typename T>
    void setSpriteSetDisplayName(SpriteSet::Type type, Uint16 spriteSetID,
                                 const std::string& newDisplayName);

    /**
     * Implementation for setSpriteSetSlot().
     */
    template<typename T>
    void setSpriteSetSlot(SpriteSet::Type type, Uint16 spriteSetID,
                          std::size_t index, int newSpriteID);

    // Save functions.
    void saveFloors(nlohmann::json& json);
    void saveFloorCoverings(nlohmann::json& json);
    void saveWalls(nlohmann::json& json);
    void saveObjects(nlohmann::json& json);

    DataModel& dataModel;

    /** Maps floor IDs -> the floor sprite sets that we currently have loaded. */
    std::map<Uint16, EditorFloorSpriteSet> floorMap;

    /** Maps floor covering IDs -> the floor sprite sets that we currently have 
        loaded. */
    std::map<Uint16, EditorFloorCoveringSpriteSet> floorCoveringMap;

    /** Maps wall IDs -> the wall sprite sets that we currently have loaded. */
    std::map<Uint16, EditorWallSpriteSet> wallMap;

    /** Maps object IDs -> the object sprite sets that we currently have loaded. */
    std::map<Uint16, EditorObjectSpriteSet> objectMap;

    /** Used for generating temporary sprite sheet IDs that are only used
        internally by this editor. */
    IDPool floorIDPool;
    IDPool floorCoveringIDPool;
    IDPool wallIDPool;
    IDPool objectIDPool;

    /** If one of our parsing functions returns false, this holds a string 
        describing the error that occurred. */
    std::string errorString;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(Uint16 floorID, const EditorFloorSpriteSet& floor)>
        floorAddedSig;
    entt::sigh<void(Uint16 floorCoveringID,
                    const EditorFloorCoveringSpriteSet& floorCovering)>
        floorCoveringAddedSig;
    entt::sigh<void(Uint16 wallID, const EditorWallSpriteSet& wall)>
        wallAddedSig;
    entt::sigh<void(Uint16 objectID, const EditorObjectSpriteSet& object)>
        objectAddedSig;

    entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID)>
        spriteSetRemovedSig;
    entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID, std::size_t index,
                    int newSpriteID)>
        spriteSetSlotChangedSig;
    entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID,
                               const std::string& newDisplayName)>
        spriteSetDisplayNameChangedSig;
};

} // namespace SpriteEditor
} // namespace AM
