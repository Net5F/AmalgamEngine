#pragma once

#include "Camera.h"
#include "TileExtent.h"
#include "AUI/Window.h"

namespace AM
{
class EventDispatcher;

namespace Client
{
class WorldSinks;
class Sprite;

/**
 * The build mode overlay on the main screen. Allows the user to place tiles
 * in the world.
 *
 * This overlay is opened alongside BuildPanel when we enter build mode.
 */
class BuildOverlay : public AUI::Window
{
public:
    //-------------------------------------------------------------------------
    // Public interface
    //-------------------------------------------------------------------------
    BuildOverlay(WorldSinks& inWorldSinks, EventDispatcher& inUiEventDispatcher);

    /**
     * Used by the BuildPanel to tell us which sprite is selected.
     *
     * The currently selected tile will follow the mouse cursor while it moves
     * within this overlay. When the mouse is clicked, the sim will be notified
     * that it needs to place the selected tile at the selected layer.
     */
    void setSelectedTile(const Sprite& inSelectedTile);

    /**
     * Used by the BuildPanel to tell us which sprite layer is selected.
     *
     * When the mouse is clicked, the sim will be notified that it needs to
     * place the selected tile at the selected layer.
     */
    void setSelectedLayer(unsigned int inTileLayerIndex);

    /**
     * Sets the camera to use when rendering.
     *
     * Called by the renderer to give us the lerped camera for the current
     * frame.
     */
    void setCamera(const Camera& inCamera);

    //-------------------------------------------------------------------------
    // Widget class overrides
    //-------------------------------------------------------------------------
    void render() override;

    AUI::EventResult onMouseDown(AUI::MouseButtonType buttonType, const SDL_Point& cursorPosition) override;

    AUI::EventResult onMouseMove(const SDL_Point& cursorPosition) override;

private:
    /**
     * Sets mapTileExtent to the new extent of the tile map.
     */
    void onTileMapExtentChanged(TileExtent inTileExtent);

    /** Used to send TileUpdateRequest events when a tile is selected. */
    EventDispatcher& uiEventDispatcher;

    /** The currently selected tile. */
    const Sprite* selectedTile;

    /** The layer to place the selected tile at. */
    unsigned int tileLayerIndex;

    /** The camera to use when rendering or doing screen -> world calcs. */
    Camera camera;

    /** The world tile map's extent. Used to make sure we aren't rendering or
        requesting changes to tiles that are out of bounds. */
    TileExtent mapTileExtent;

    /** The position of the tile that the mouse is hovering over.
        The currently selected tile will be rendered semi-transparently at
        this position. */
    TilePosition mouseTilePosition;
};

} // End namespace Client
} // End namespace AM
