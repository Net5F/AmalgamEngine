#include <AssetManager.h>

std::string NW::AssetManager::StripPath (const std::string& Path) const
{
    int lastSlashPosition = Path.find_last_of("/\\");

    if (lastSlashPosition == std::string::npos)
        return Path;
    else
        return Path.substr(lastSlashPosition + 1);
}

bool NW::AssetManager::LoadTexture (const std::string& Path)
{
    try
    {
        // Save the texture
        textureStore.insert_or_assign(StripPath(Path)
                                      , SDL2pp::Texture(renderer, Path));
    }
    catch(SDL2pp::Exception& e)
    {
        // Likely an invalid path
        std::cerr << "Error in: " << e.GetSDLFunction() << std::endl;
        std::cerr << "  Reason:  " << e.GetSDLError() << std::endl;
        return false;
    }

    return true;
}

const SDL2pp::Texture& NW::AssetManager::GetTexture (
        const std::string& FileName) const
{
    const SDL2pp::Texture& foundTexture = textureStore.find(FileName)->second;

    return foundTexture;
}

NW::AssetManager::AssetManager (SDL2pp::Renderer& InRenderer) :
        renderer(InRenderer)
{
}
