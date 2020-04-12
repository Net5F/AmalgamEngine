#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <AssetManager.h>
#include <RenderManager.h>
#include <TileManager.h>
#include <SDL2pp/SDL2pp.hh>

namespace NW
{

/**
 * \class GameManager
 *
 * \brief Inits and maintains window, runs the game.
 *
 * This class is the entry point for the game.
 * It should be instantiated once in main() and
 * told to Start().
 *
 * General game features like time tracking,
 * loop management, and window management are
 * handled here.
 *
 * This class also instantiates Game submodule
 * constructs and facilitates their communication
 * with other engine libraries (e.g. TileManager
 * needing to talk to the AssetManager).
 *
 *
 * \author Michael Puskas
 *
 * Created on: Dec 31, 2018
 *
 */
class GameManager
{
private:
    /**********************************/
    /* SDL constructs
    /**********************************/
    SDL2pp::SDL sdl;

    SDL2pp::Window window;

    SDL2pp::Renderer renderer;


    /**********************************/
    /* Graphics constructs
    /**********************************/
    AssetManager assetManager;

    RenderManager renderManager;


    /**********************************/
    /* Game constructs
    /**********************************/
    TileManager tileManager;


    /**********************************/
    /* Private member variables
    /**********************************/
    /* Screen size (in pixels). */
    XYVec screenSize;

    /* Viewport size (in tiles). */
    RCVec viewportSize;

    /* Our viewport's current top left coordinate within
       the tile map. Can be negative. */
    RCVec viewportTopLeft;


    /**********************************/
    /* Private member functions
    /**********************************/
    /**
     * Wraps Tick() with the main game loop.
     */
    void Run();

    /**
     * Calls functions to handle input events,
     * update game state, and render.
     */
    void Tick();

    /**
     * Clears the buffer, calls RenderManager to copy
     * the tiles and actors to the screen, then
     * pushes the back buffer to the front.
     */
    void Render();

public:
    /**
     * Initializes game logic and starts main loop.
     *
     * \throw SDL2pp::Exception for any SDL2pp exceptions.
     * \throw std::exception for all other exceptions.
     */
    void Start();


    /**********************************/
    /* Convenience functions
    /**********************************/


    /******************/
    /*    Getters     */
    /******************/
    const SDL2pp::SDL& GetSdl() const;
    const SDL2pp::Window& GetWindow() const;
    const SDL2pp::Renderer& GetRenderer() const;
    const AssetManager& GetAssetManager() const;
    const RenderManager& GetRenderManager() const;


    /**
     * Prepares SDL constructs for game start.
     */
    GameManager(const std::string& Title
                , XYVec InScreenSize
                , RCVec InViewportSize);

};

} /* End namespace NW */

#endif /* End GAMEMANAGER_H */
