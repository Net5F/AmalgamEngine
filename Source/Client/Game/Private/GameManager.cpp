#include <GameManager.h>

void NW::GameManager::Start() try
{
    /* Load Assets */
    assetManager.LoadTexture("Resources/u4_tiles_pc_ega.png");

    /* Construct sprites */

    /* Parse and build tiles */
    tileManager.BuildTileTypes("DummyFile");

    /* Parse and build tile map */
    tileManager.BuildTileMap("DummyFile");

    /* Pass the map size to the render manager */
    renderManager.SetMapSize(tileManager.GetMapSize());

    /* Start the main loop */
    Run();
}
catch (SDL2pp::Exception& e)
{
    std::cerr << "Error in: " << e.GetSDLFunction() << std::endl;
    std::cerr << "  Reason:  " << e.GetSDLError() << std::endl;
    return;
}
catch (std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return;
}
catch (...)
{
    std::cerr << "Generic error bubbled to NW::GameManager::Start()"
            << std::endl;
}

void NW::GameManager::Run()
{
    bool bQuit = false;

    while (!bQuit)
    {
        /* Update player input */
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                bQuit = true;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        bQuit = true;
                }
            }
        }

        /* Update game state */
        Tick();
    }
}

void NW::GameManager::Tick()
{
    // TODO: Split into process input, update game state, render

    Render();

    SDL_Delay(1);
}

void NW::GameManager::Render()
{
    renderer.Clear();

    /* Render the map tiles */
    renderManager.CopyTiles(viewportTopLeft
            , (const Sprite * const *) tileManager.GetTileMap());

    renderer.Present();
}

const SDL2pp::SDL& NW::GameManager::GetSdl() const
{
    return sdl;
}

const SDL2pp::Window& NW::GameManager::GetWindow() const
{
    return window;
}

const SDL2pp::Renderer& NW::GameManager::GetRenderer() const
{
    return renderer;
}

const NW::AssetManager& NW::GameManager::GetAssetManager() const
{
    return assetManager;
}

const NW::RenderManager& NW::GameManager::GetRenderManager() const
{
    return renderManager;
}

NW::GameManager::GameManager(const std::string& Title
                         , XYVec InScreenSize, RCVec InViewportSize) :
        sdl(SDL_INIT_VIDEO)
        , window(Title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED
                 , InScreenSize.xVal, InScreenSize.yVal, SDL_WINDOW_SHOWN)
        , renderer(window, -1, SDL_RENDERER_ACCELERATED)
        , assetManager(renderer)
        , renderManager(renderer, InScreenSize
                        , {0, 0} // Set this after building the map.
                        , InViewportSize)
        , tileManager(*this)
        , screenSize{InScreenSize}
        , viewportSize{InViewportSize}
        , viewportTopLeft{0, 0}
{
}
