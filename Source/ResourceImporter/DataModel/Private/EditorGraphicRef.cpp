#include "EditorGraphicRef.h"
#include "BoundingBoxModel.h"
#include "VariantTools.h"
#include "Log.h"

namespace AM
{
namespace ResourceImporter
{

GraphicID EditorGraphicRef::getGraphicID() const
{
    GraphicID graphicID{NULL_GRAPHIC_ID};
    std::visit(VariantTools::Overload{
        [&](std::reference_wrapper<const EditorSprite> sprite) {
            graphicID = toGraphicID(sprite.get().numericID);
        },
        [&](std::reference_wrapper<const EditorAnimation> animation) {
            graphicID = toGraphicID(animation.get().numericID);
        }
    }, *this);

    return graphicID;
}

const std::string& EditorGraphicRef::getDisplayName() const
{
    const std::string* displayName{};
    std::visit(
        [&](const auto& underlying) {
            displayName = &(underlying.get().displayName);
        },
        *this);

    return *displayName;
}

bool EditorGraphicRef::getCollisionEnabled() const
{
    bool collisionEnabled{false};
    std::visit(
        [&](const auto& underlying) {
            collisionEnabled = underlying.get().collisionEnabled;
        },
        *this);

    return collisionEnabled;
}

const BoundingBox& EditorGraphicRef::getModelBounds(
    const BoundingBoxModel& boundingBoxModel) const
{
    const BoundingBox* modelBounds{nullptr};
    std::visit(
        [&](const auto& underlying) {
            modelBounds = &(underlying.get().getModelBounds(boundingBoxModel));
        },
        *this);

    return *modelBounds;
}

const EditorSprite* EditorGraphicRef::getFirstSprite() const
{
    const EditorSprite* spritePtr{nullptr};
    std::visit(VariantTools::Overload{
        [&](std::reference_wrapper<const EditorSprite> sprite) {
            spritePtr = &(sprite.get());
        },
        [&](std::reference_wrapper<const EditorAnimation> animation) {
            if (animation.get().frames.size() > 0) {
                spritePtr = &(animation.get().frames[0].sprite.get());
            }
        }
    }, *this);

    return spritePtr;
}

const EditorSprite* EditorGraphicRef::getSpriteAtTime(double animationTime) const
{
    const EditorSprite* spritePtr{nullptr};
    std::visit(VariantTools::Overload{
        [&](std::reference_wrapper<const EditorSprite> sprite) {
            spritePtr = &(sprite.get());
        },
        [&](std::reference_wrapper<const EditorAnimation> animation) {
            spritePtr = animation.get().getSpriteAtTime(animationTime);
        }
    }, *this);

    return spritePtr;
}

} // End namespace ResourceImporter
} // End namespace AM
