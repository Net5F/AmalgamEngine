#pragma once

#include "GraphicSets.h"
#include "EditorFloorGraphicSet.h"
#include "EditorTerrainGraphicSet.h"
#include "EditorWallGraphicSet.h"
#include "EditorObjectGraphicSet.h"
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
 * Holds data for Floor, Floor Covering, Wall, and Object graphic sets.
 * Entity graphic sets use EntityGraphicSetModel.
 */
class GraphicSetModel
{
public:
    GraphicSetModel(DataModel& inDataModel);

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
     * Adds a blank graphic set of the appropriate type and loads it.
     */
    bool addTerrain();
    bool addFloor();
    bool addWall();
    bool addObject();

    /**
     * Removes the graphic set of the appropriate type with the given ID from
     * the associated map.
     *
     * Error if the given ID isn't present in the map.
     *
     * @param graphicSetID  The editor ID of the graphic set to remove.
     */
    void remTerrain(TerrainGraphicSetID terrainID);
    void remFloor(FloorGraphicSetID floorCoveringID);
    void remWall(WallGraphicSetID wallID);
    void remObject(ObjectGraphicSetID objectID);

    const EditorTerrainGraphicSet& getTerrain(TerrainGraphicSetID terrainID);
    const EditorFloorGraphicSet&
        getFloor(FloorGraphicSetID floorID);
    const EditorWallGraphicSet& getWall(WallGraphicSetID wallID);
    const EditorObjectGraphicSet& getObject(ObjectGraphicSetID objectID);

    // Graphic set properties.
    void setGraphicSetDisplayName(GraphicSet::Type type, Uint16 graphicSetID,
                                 const std::string& newDisplayName);
    void setGraphicSetSlot(GraphicSet::Type type, Uint16 graphicSetID,
                          std::size_t index, GraphicID newGraphicID);

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
     * @param graphicSetJson  The json to parse. Must be a valid graphic set
     *                        section from ResourceData.json, for the appropriate
     *                        set type.
     */
    bool parseTerrainGraphicSet(const nlohmann::json& graphicSetJson);
    bool parseFloorGraphicSet(const nlohmann::json& graphicSetJson);
    bool parseWallGraphicSet(const nlohmann::json& graphicSetJson);
    bool parseObjectGraphicSet(const nlohmann::json& graphicSetJson);

    /**
     * Returns the appropriate map for the given graphic set type.
     */
    template<typename T>
    std::map<Uint16, T>& getMapForGraphicSetType();

    /**
     * Checks if the given name is unique among all graphic sets of the same 
     * type in the model.
     *
     * @param graphicSetID  The ID of the graphic set that might get displayName.
     *                      If it already is set to displayName, it won't be 
     *                      counted as non-unique.
     * @param displayName  The display name that the graphic set will be set to.
     */
    template<typename T>
    bool graphicSetNameIsUnique(Uint16 graphicSetID,
                               const std::string& displayName);

    /**
     * Implementation for setGraphicSetDisplayName().
     */
    template<typename T>
    void setGraphicSetDisplayName(GraphicSet::Type type, Uint16 graphicSetID,
                                 const std::string& newDisplayName);

    /**
     * Implementation for setGraphicSetSlot().
     */
    template<typename T>
    void setGraphicSetSlot(GraphicSet::Type type, Uint16 graphicSetID,
                          std::size_t index, GraphicID newGraphicID);

    // Save functions.
    void saveTerrain(nlohmann::json& json);
    void saveFloors(nlohmann::json& json);
    void saveWalls(nlohmann::json& json);
    void saveObjects(nlohmann::json& json);

    DataModel& dataModel;

    // Note: These all use Uint16 instead of the specific ID type, so we can 
    //       interact with them generically.
    /** Maps terrain IDs -> the terrain graphic sets that we currently have 
        loaded. */
    std::map<Uint16, EditorTerrainGraphicSet> terrainMap;

    /** Maps floor IDs -> the floor graphic sets that we currently have loaded. 
     */
    std::map<Uint16, EditorFloorGraphicSet> floorMap;

    /** Maps wall IDs -> the wall graphic sets that we currently have loaded. */
    std::map<Uint16, EditorWallGraphicSet> wallMap;

    /** Maps object IDs -> the object graphic sets that we currently have loaded.
     */
    std::map<Uint16, EditorObjectGraphicSet> objectMap;

    /** Used for generating temporary graphic set IDs that are only used
        internally by this editor. */
    IDPool terrainIDPool;
    IDPool floorIDPool;
    IDPool wallIDPool;
    IDPool objectIDPool;

    /** If one of our parsing functions returns false, this holds a string
        describing the error that occurred. */
    std::string errorString;

    //-------------------------------------------------------------------------
    // Signals
    //-------------------------------------------------------------------------
    entt::sigh<void(TerrainGraphicSetID floorID,
                    const EditorTerrainGraphicSet& floor)>
        terrainAddedSig;
    entt::sigh<void(FloorGraphicSetID floorCoveringID,
                    const EditorFloorGraphicSet& floorCovering)>
        floorAddedSig;
    entt::sigh<void(WallGraphicSetID wallID, const EditorWallGraphicSet& wall)>
        wallAddedSig;
    entt::sigh<void(ObjectGraphicSetID objectID,
                    const EditorObjectGraphicSet& object)>
        objectAddedSig;

    entt::sigh<void(GraphicSet::Type type, Uint16 graphicSetID)>
        graphicSetRemovedSig;
    entt::sigh<void(GraphicSet::Type type, Uint16 graphicSetID, std::size_t index,
                    GraphicID newGraphicID)>
        graphicSetSlotChangedSig;
    entt::sigh<void(GraphicSet::Type type, Uint16 graphicSetID,
                    const std::string& newDisplayName)>
        graphicSetDisplayNameChangedSig;

public:
    //-------------------------------------------------------------------------
    // Signal Sinks
    //-------------------------------------------------------------------------
    /** A terrain graphic set was added to the model. */
    entt::sink<
        entt::sigh<void(TerrainGraphicSetID terrainID,
                        const EditorTerrainGraphicSet& terrain)>>
        terrainAdded;
    /** A floor graphic set was added to the model. */
    entt::sink<entt::sigh<void(FloorGraphicSetID floorID,
                               const EditorFloorGraphicSet& floor)>>
        floorAdded;
    /** A wall graphic set was added to the model. */
    entt::sink<entt::sigh<void(WallGraphicSetID wallID,
                               const EditorWallGraphicSet& wall)>>
        wallAdded;
    /** An object graphic set was added to the model. */
    entt::sink<entt::sigh<void(ObjectGraphicSetID objectID,
                               const EditorObjectGraphicSet& floor)>>
        objectAdded;

    /** A graphic set was removed from the model. */
    entt::sink<entt::sigh<void(GraphicSet::Type type, Uint16 graphicSetID)>>
        graphicSetRemoved;

    /** A graphic set's graphic at the given index was changed. */
    entt::sink<entt::sigh<void(GraphicSet::Type type, Uint16 graphicSetID,
                               std::size_t index, GraphicID newGraphicID)>>
        graphicSetSlotChanged;

    /** A graphic set's display name has changed. */
    entt::sink<entt::sigh<void(GraphicSet::Type type, Uint16 graphicSetID,
                               const std::string& newDisplayName)>>
        graphicSetDisplayNameChanged;
};

} // namespace ResourceImporter
} // namespace AM
