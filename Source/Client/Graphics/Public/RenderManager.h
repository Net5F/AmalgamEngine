#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include <NWStructs.h>
#include <SDL2pp/SDL2pp.hh>
#include <memory>

namespace NW
{

/**********************************/
/* Forward declarations
/**********************************/
class Sprite;


/**
 * \class RenderManager
 * 
 * \brief Maintains screen size and renders on request.
 * 
 * This class is in charge of rendering sprites to the screen.
 *
 *
 * \author Michael Puskas
 *
 * Created on: Jan 3, 2019
 *
 */

class RenderManager
{
private:
    SDL2pp::Renderer& renderer;

    /* Screen size (in pixels).

       When the screen size is updated for the window,
       this needs to be updated. */
    XYVec screenSize;

    /* Map size (in tiles).

       Must be updated when the map changes, such
       as on server switching. */
    RCVec mapSize;

    /* Viewport size (in tiles).

       Change to alter the zoom distance. */
    RCVec viewportSize;

    /* Based on viewport and screen size. */
    XYVec spriteSize;

public:
    /**
     * Copies the given tiles to the back buffer.
     * Does not clear or present the buffer.
     *
     * \param[in] TopLeftCoord  The screen's current top left coordinate.
     * \param[in] MapPtr  A pointer to the TileManager's tile map.
     */
    void CopyTiles(const RCVec& ViewportTopLeft
                   , const Sprite * const * MapPtr);

    void SetScreenSize(XYVec InScreenSize);

    void SetMapSize(RCVec InMapSize);

    void SetViewportSize(RCVec InViewportSize);


    /**
     * Saves the renderer pointer locally, initializes render info.
     */
    RenderManager(SDL2pp::Renderer& InRenderer
                  , XYVec InScreenSize
                  , RCVec InMapSize
                  , RCVec InViewportSize);

};

} /* End namespace NW */

#endif /* End RENDERMANAGER_H  */
