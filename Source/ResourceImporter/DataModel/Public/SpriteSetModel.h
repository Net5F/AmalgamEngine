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
namespace ResourceImporter
{

class DataModel;

/**
 * Holds data for each type of sprite set (floor, floor covering, wall, and
 * object).
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
     * Error if the given ID isn't present in the map.
     *
     * @param spriteSetID  The editor ID of the sprite set to remove.
     */
    void remFloor(FloorSpriteSetID floorID);
    void remFloorCovering(FloorCoveringSpriteSetID floorCoveringID);
    void remWall(WallSpriteSetID wallID);
    void remObject(ObjectSpriteSetID objectID);

    const EditorFloorSpriteSet& getFloor(FloorSpriteSetID floorID);
    const EditorFloorCoveringSpriteSet&
        getFloorCovering(FloorCoveringSpriteSetID floorCoveringID);
    const EditorWallSpriteSet& getWall(WallSpriteSetID wallID);
    const EditorObjectSpriteSet& getObject(ObjectSpriteSetID objectID);

    // Sprite set properties.
    void setSpriteSetDisplayName(SpriteSet::Type type, Uint16 spriteSetID,
                                 const std::string& newDisplayName);
    void setSpriteSetSlot(SpriteSet::Type type, Uint16 spriteSetID,
                          std::size_t index, SpriteID newSpriteID);

    /** Resets the model state, setting it back to default. */
    void resetModelState();

    /**
     * Iterates all the sprite sets and replaces any instances of spriteID
     * with NULL_SPRITE_ID.
     */
    void removeSpriteIDFromSets(SpriteID spriteID);

    const std::string& getErrorString();

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
                          std::size_t index, SpriteID newSpriteID);

    // Save functions.
    void saveFloors(nlohmann::json& json);
    void saveFloorCoverings(nlohmann::json& json);
    void saveWalls(nlohmann::json& json);
    void saveObjects(nlohmann::json& json);

    DataModel& dataModel;

    // Note: These all use Uint16 instead of the specific ID type, so we can 
    //       interact with them generically.
    /** Maps floor IDs -> the floor sprite sets that we currently have loaded.
     */
    std::map<Uint16, EditorFloorSpriteSet> floorMap;

    /** Maps floor covering IDs -> the floor sprite sets that we currently have
        loaded. */
    std::map<Uint16, EditorFloorCoveringSpriteSet> floorCoveringMap;

    /** Maps wall IDs -> the wall sprite sets that we currently have loaded. */
    std::map<Uint16, EditorWallSpriteSet> wallMap;

    /** Maps object IDs -> the object sprite sets that we currently have loaded.
     */
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
    entt::sigh<void(FloorSpriteSetID floorID,
                    const EditorFloorSpriteSet& floor)>
        floorAddedSig;
    entt::sigh<void(FloorCoveringSpriteSetID floorCoveringID,
                    const EditorFloorCoveringSpriteSet& floorCovering)>
        floorCoveringAddedSig;
    entt::sigh<void(WallSpriteSetID wallID, const EditorWallSpriteSet& wall)>
        wallAddedSig;
    entt::sigh<void(ObjectSpriteSetID objectID,
                    const EditorObjectSpriteSet& object)>
        objectAddedSig;

    entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID)>
        spriteSetRemovedSig;
    entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID, std::size_t index,
                    SpriteID newSpriteID)>
        spriteSetSlotChangedSig;
    entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID,
                    const std::string& newDisplayName)>
        spriteSetDisplayNameChangedSig;

public:
    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** A floor sprite set was added to the model. */
    entt::sink<entt::sigh<void(FloorSpriteSetID floorID,
                               const EditorFloorSpriteSet& floor)>>
        floorAdded;
    /** A floor covering sprite set was added to the model. */
    entt::sink<
        entt::sigh<void(FloorCoveringSpriteSetID floorCoveringID,
                        const EditorFloorCoveringSpriteSet& floorCovering)>>
        floorCoveringAdded;
    /** A wall sprite set was added to the model. */
    entt::sink<entt::sigh<void(WallSpriteSetID wallID,
                               const EditorWallSpriteSet& wall)>>
        wallAdded;
    /** An object sprite set was added to the model. */
    entt::sink<entt::sigh<void(ObjectSpriteSetID objectID,
                               const EditorObjectSpriteSet& floor)>>
        objectAdded;

    /** A sprite set was removed from the model. */
    entt::sink<entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID)>>
        spriteSetRemoved;

    /** A sprite set's sprite at the given index was changed. */
    entt::sink<entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID,
                               std::size_t index, SpriteID newSpriteID)>>
        spriteSetSlotChanged;

    /** A sprite set's display name has changed. */
    entt::sink<entt::sigh<void(SpriteSet::Type type, Uint16 spriteSetID,
                               const std::string& newDisplayName)>>
        spriteSetDisplayNameChanged;
};

} // namespace ResourceImporter
} // namespace AM
