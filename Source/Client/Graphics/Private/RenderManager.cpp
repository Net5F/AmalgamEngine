#include <RenderManager.h>
#include <Sprite.h>
#include <iostream>

void NW::RenderManager::CopyTiles(const RCVec& ViewportTopLeft
                                  , const Sprite * const * MapPtr)
{
    for (int i = ViewportTopLeft.rowVal;
            i < (ViewportTopLeft.rowVal + mapSize.rowVal); ++i)
    {
        for (int j = ViewportTopLeft.columnVal;
                j < (ViewportTopLeft.columnVal + mapSize.columnVal); ++j)
        {
            int currentIndex = (i * mapSize.columnVal) + j;

            // Won't draw if the tile is out of bounds or empty.
            if (j < mapSize.columnVal && i < mapSize.rowVal
                    && MapPtr[currentIndex] != nullptr)
            {
                // (Relative position) * sprite size
                int xPosition = (j - ViewportTopLeft.columnVal)
                                * spriteSize.xVal;
                int yPosition = (i - ViewportTopLeft.rowVal)
                                * spriteSize.yVal;

                renderer.Copy(const_cast<SDL2pp::Texture&>(
                              MapPtr[currentIndex]->GetSourceTexture())
                              , MapPtr[currentIndex]->GetSourceRect()
                              , SDL2pp::Rect(xPosition, yPosition
                                         , spriteSize.xVal, spriteSize.yVal));
            }
        }
    }

    // TODO: Move character rendering to a separate function
    int centerX = renderer.GetOutputWidth() / 2;
    int centerY = renderer.GetOutputHeight() / 2;

    const SDL2pp::Texture& spriteTexture = MapPtr[0]->GetSourceTexture();

    // First rect is in UV (texture space), second is in ST (world space)
    auto dudeSrcRect = SDL2pp::Rect(0, 32, 16, 16);
    auto dudeDestRect = SDL2pp::Rect(centerX - spriteSize.xVal
                                     , centerY - spriteSize.yVal
                                     , spriteSize.xVal, spriteSize.yVal);

    renderer.Copy(const_cast<SDL2pp::Texture&>(spriteTexture)
                  , dudeSrcRect, dudeDestRect);
}

void NW::RenderManager::SetScreenSize(XYVec InScreenSize)
{
    screenSize = InScreenSize;

    /* Update sprite size */
    spriteSize = {(screenSize.xVal / viewportSize.columnVal)
                      , (screenSize.yVal / viewportSize.rowVal)};
}

void NW::RenderManager::SetMapSize(RCVec InMapSize)
{
    mapSize = InMapSize;
}

void NW::RenderManager::SetViewportSize(RCVec InViewportSize)
{
    viewportSize = InViewportSize;

    /* Update sprite size */
    spriteSize = {(screenSize.xVal / viewportSize.columnVal)
                      , (screenSize.yVal / viewportSize.rowVal)};
}

NW::RenderManager::RenderManager(SDL2pp::Renderer& InRenderer
        , XYVec InScreenSize, RCVec InMapSize, RCVec InViewportSize) :
        renderer(InRenderer)
        , screenSize{InScreenSize}
        , mapSize{InMapSize}
        , viewportSize{InViewportSize}
        , spriteSize{(InScreenSize.xVal / InViewportSize.columnVal)
                      , (InScreenSize.yVal / InViewportSize.rowVal)}
{
}
