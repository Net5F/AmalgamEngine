#include "SpriteTools.h"
#include "DataModel.h"
#include "EditorSpriteSheet.h"
#include "Position.h"
#include "Transforms.h"
#include "Camera.h"
#include "Log.h"
#include "AUI/Core.h"
#include <SDL_render.h>
#include <SDL_image.h>

namespace AM
{
namespace ResourceImporter
{
BoundingBox 
SpriteTools::calcSpriteStageWorldExtent(const SDL_Rect& spriteImageExtent,
    const SDL_Point& stageOrigin)
{
    // There are 3 options for how we could've sized the stage:
    //   1. Clamp max values to stay within the image bounds (would form an 
    //      irregular shape in-world).
    //   2. Form a world-space axis-aligned box starting at the origin, make it
    //      as large as possible while never leaving the image bounds.
    //   3. Same as #2, but make it oversized so the whole image (below the 
    //      origin) is within the stage.
    // We chose to go with #3, because it lets us use the whole image, and 
    // is easier to implement than #1. The downside is that it lets you draw 
    // boxes that go outside of the image, but that's easy to avoid if you 
    // don't want to.

    // Find the screen-space position of the images bottom corners, relative 
    // to the origin.
    SDL_FPoint bottomLeft{
        static_cast<float>(-1.f * stageOrigin.x),
        static_cast<float>(spriteImageExtent.h - stageOrigin.y)};
    SDL_FPoint bottomRight{
        static_cast<float>(spriteImageExtent.w - stageOrigin.x),
        static_cast<float>(spriteImageExtent.h - stageOrigin.y)};

    // Convert the screen-space points to world space.
    Position worldBottomLeft{Transforms::screenToWorldMinimum(bottomLeft, {})};
    Position worldBottomRight{
        Transforms::screenToWorldMinimum(bottomRight, {})};

    // Calculate the stage's extent from the boundary points.
    BoundingBox stageWorldExtent{{0, 0, 0}, {0, 0, 0}};
    stageWorldExtent.max.x = worldBottomRight.x;
    stageWorldExtent.max.y = worldBottomLeft.y;
    // Z must span from the lowest point on the screen (maxX, maxY, 0), to 
    // the top of the image.
    float originToMaxY{Transforms::worldToScreen(
        {worldBottomRight.x, worldBottomLeft.y, 0}, 1.f).y};
    stageWorldExtent.max.z = Transforms::screenYToWorldZ(
        (static_cast<float>(stageOrigin.y) + originToMaxY), 1.0);

    return stageWorldExtent;
}

SDL_Texture* SpriteTools::generateSpriteSheetTexture(
    const DataModel& dataModel, const EditorSpriteSheet& spriteSheet)
{
    // Create an empty texture to hold the sprite sheet.
    // Note: We check that ARGB8888 is supported in SpriteModel.
    SDL_RendererInfo info{};
    SDL_GetRendererInfo(AUI::Core::getRenderer(), &info);
    SDL_Texture* spriteSheetTexture{
        SDL_CreateTexture(AUI::Core::getRenderer(), SDL_PIXELFORMAT_ARGB8888,
                          SDL_TEXTUREACCESS_TARGET, spriteSheet.textureWidth,
                          spriteSheet.textureHeight)};

    // Set the blend mode (default is NONE, which causes black backgrounds).
    SDL_SetTextureBlendMode(spriteSheetTexture, SDL_BLENDMODE_BLEND);

    // Set the texture as the render target.
    SDL_Texture* previousRenderTarget{
        SDL_GetRenderTarget(AUI::Core::getRenderer())};
    SDL_SetRenderTarget(AUI::Core::getRenderer(), spriteSheetTexture);

    // Copy all of the sprites into the sprite sheet texture.
    std::string fullImagePath{};
    for (SpriteID spriteID : spriteSheet.spriteIDs) {
        const EditorSprite& sprite{dataModel.spriteModel.getSprite(spriteID)};

        // Load the sprite's texture.
        fullImagePath = dataModel.getWorkingIndividualSpritesDir();
        fullImagePath += sprite.imagePath;
        SDL_Texture* spriteTexture{};
        if (sprite.premultiplyAlpha) {
            spriteTexture
                = loadAndPremultiplyTexture(fullImagePath);
        }
        else {
            spriteTexture = IMG_LoadTexture(AUI::Core::getRenderer(),
                                            fullImagePath.c_str());
        }

        if (!spriteTexture) {
            LOG_INFO("Failed to load texture: %s", fullImagePath.c_str());
            break;
        }

        // Disable blending, so the sprite's pixels are copied exactly as-is.
        SDL_SetTextureBlendMode(spriteTexture, SDL_BLENDMODE_NONE);

        // Copy the sprite into the sheet texture;
        SDL_Rect sourceRect{0, 0, sprite.textureExtent.w,
                            sprite.textureExtent.h};
        SDL_RenderCopy(AUI::Core::getRenderer(), spriteTexture, &sourceRect,
                       &(sprite.textureExtent));

        // Clean up the sprite texture.
        SDL_DestroyTexture(spriteTexture);
    }

    // Set the render target back to what it was.
    SDL_SetRenderTarget(AUI::Core::getRenderer(), previousRenderTarget);

    return spriteSheetTexture;
}

SDL_Texture*
    SpriteTools::loadAndPremultiplyTexture(const std::string& fullImagePath)
{
    // Load the image as a surface so we can manipulate it.
    SDL_Surface* spriteSurface{IMG_Load(fullImagePath.c_str())};
    if (!spriteSurface) {
        return nullptr;
    }

    // If the surface isn't ARGB8888, convert it.
    if (spriteSurface->format->format != SDL_PIXELFORMAT_ARGB8888) {
        SDL_Surface* newSurface{SDL_ConvertSurfaceFormat(
            spriteSurface, SDL_PIXELFORMAT_ARGB8888, 0)};
        SDL_FreeSurface(spriteSurface);
        spriteSurface = newSurface;
    }

    // Premultiply the alpha channel into the color channels.
    int pitch{spriteSurface->w * 4};
    if (SDL_PremultiplyAlpha(spriteSurface->w, spriteSurface->h,
                             SDL_PIXELFORMAT_ARGB8888, spriteSurface->pixels,
                             pitch, SDL_PIXELFORMAT_ARGB8888,
                             spriteSurface->pixels, pitch)
        != 0) {
        LOG_INFO("Error while premultiplying alpha: %s", SDL_GetError());
    }

    // Turn the surface into a texture and return it.
    SDL_Texture* spriteTexture{
        SDL_CreateTextureFromSurface(AUI::Core::getRenderer(), spriteSurface)};
    SDL_FreeSurface(spriteSurface);
    return spriteTexture;
}

} // End namespace ResourceImporter
} // End namespace AM
