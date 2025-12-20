#pragma once

#include "Rotation.h"
#include <vector>

namespace AM
{
struct Vector3;
struct AVEntity;
struct Position;
struct PreviousPosition;
struct GraphicState;

namespace Client
{
struct SimulationContext;
class World;
class GraphicData;
struct VisualEffectState;
struct AVEntityState;
struct ClientGraphicState;

/**
 * Updates audio/visual effects and entities.
 *
 * A/V effects are attached to a particular entity, and follow that entity 
 * around for a set period of time. These are used for e.g. a power-up effect.
 *
 * A/V entities are spawned as local-only entities, and have a set of logic 
 * to perform before being destroyed. These are used for e.g. a blizzard spell.
 */
class AVSystem
{
public:
    AVSystem(const SimulationContext& inSimContext);

    /**
     * Updates A/V effects and entities.
     */
    void updateAVEffectsAndEntities();

private:
    /**
     * Updates all AVEffects components.
     */
    void updateAVEffects();

    /**
     * Updates all of the given visual effects.
     */
    void updateVisualEffects(std::vector<VisualEffectState>& visualEffects);

    /**
     * Updates all A/V entities.
     */
    void updateAVEntities();

    /**
     * If the given phase is completed, increments to the next phase.
     * @return false if the update was invalid and the entity should be deleted.
     */
    bool incrementPhaseIfNecessary(AVEntityState& avEntityState,
                                   const Position& position,
                                   const GraphicState& graphicState,
                                   ClientGraphicState& clientGraphicState,
                                   double currentTime);

    /**
     * Updates the given A/V entity to the next tick.
     * @return false if the update was invalid and the entity should be deleted.
     */
    bool updateAVEntity(const AVEntityState& avEntityState,
                        Position& position,
                        PreviousPosition& previousPosition,
                        GraphicState& graphicState,
                        ClientGraphicState& clientGraphicState);

    /** Used to access entity data. */
    World& world;
    const GraphicData& graphicData;
};

} // End namespace Client
} // End namespace AM
