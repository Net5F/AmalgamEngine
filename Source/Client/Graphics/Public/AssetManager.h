#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <SDL2pp/SDL2pp.hh>
#include <unordered_map>
#include <string>

namespace NW
{

/**
 * \class AssetManager
 * 
 * \brief Loads and manages assets.
 * 
 * Loads assets from resource files.
 *
 * Maintains assets and returns
 * appropriate assets on request.
 *
 * \author Michael Puskas
 *
 * Created on: Jan 3, 2019
 *
 */
class AssetManager
{
private:
    SDL2pp::Renderer& renderer;

    /* In charge of the lifespan of its Textures. */
    std::unordered_map<std::string, SDL2pp::Texture> textureStore;

    /**
     * Given a filepath "abc/xyz/file.png", returns the filename "file.png"
     * If the filename is already in the correct form, it's returned unchanged.
     *
     * \param[in] path  A filepath.
     *
     * \return The file's name, including extension.
     */
    std::string StripPath(const std::string& Path) const;

public:
    /**
     * Loads a texture and saves it in textureStore.
     *
     * \param[in]  path   A full path from the executable's base directory.
     *
     * \return True if the asset was found and loaded, false if not.
     */
    bool LoadTexture(const std::string& Path);

    /**
     * Returns a texture pointer from the textureStore.
     *
     * \param[in]  fileName  The file name of the texture, including extension.
     *
     * \return A texture pointer if one was found, nullptr if not.
     */
    const SDL2pp::Texture& GetTexture(const std::string& FileName) const;

    /**
     * Saves the renderer pointer locally.
     */
    AssetManager(SDL2pp::Renderer& InRenderer);

};

} /* End namespace NW */

#endif /* ASSETMANAGER_H */
