#include "GraphicRef.h"
#include "VariantTools.h"
#include "Log.h"

namespace AM
{

GraphicID GraphicRef::getGraphicID() const
{
    GraphicID graphicID{NULL_GRAPHIC_ID};
    std::visit(VariantTools::Overload{
        [&](std::reference_wrapper<const Sprite> sprite) {
            graphicID = toGraphicID(sprite.get().numericID);
        },
        [&](std::reference_wrapper<const Animation> animation) {
            graphicID = toGraphicID(animation.get().numericID);
        }
    }, *this);

    return graphicID;
}

const std::string& GraphicRef::getDisplayName() const
{
    const std::string* displayName{};
    std::visit(
        [&](const auto& underlying) {
            displayName = &(underlying.get().displayName);
        },
        *this);

    return *displayName;
}

bool GraphicRef::getCollisionEnabled() const
{
    bool collisionEnabled{false};
    std::visit(
        [&](const auto& underlying) {
            collisionEnabled = underlying.get().collisionEnabled;
        },
        *this);

    return collisionEnabled;
}

const BoundingBox& GraphicRef::getModelBounds() const
{
    const BoundingBox* modelBounds{nullptr};
    std::visit(
        [&](const auto& underlying) {
            modelBounds = &(underlying.get().modelBounds);
        },
        *this);

    return *modelBounds;
}

const Sprite& GraphicRef::getFirstSprite() const
{
    const Sprite* spritePtr{nullptr};
    std::visit(VariantTools::Overload{
        [&](std::reference_wrapper<const Sprite> sprite) {
            spritePtr = &(sprite.get());
        },
        [&](std::reference_wrapper<const Animation> animation) {
            if (animation.get().frames.size() > 0) {
                spritePtr = &(animation.get().frames[0].sprite.get());
            }
        }
    }, *this);

    return *spritePtr;
}

const Sprite* GraphicRef::getSpriteAtTime(double animationTime) const
{
    const Sprite* spritePtr{nullptr};
    std::visit(VariantTools::Overload{
        [&](std::reference_wrapper<const Sprite> sprite) {
            spritePtr = &(sprite.get());
        },
        [&](std::reference_wrapper<const Animation> animation) {
            spritePtr = animation.get().getSpriteAtTime(animationTime);
        }
    }, *this);

    return spritePtr;
}

} // End namespace AM
