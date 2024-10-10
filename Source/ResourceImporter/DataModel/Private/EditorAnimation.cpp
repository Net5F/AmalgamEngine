#include "EditorAnimation.h"
#include "BoundingBoxModel.h"
#include "AMAssert.h"

namespace AM
{
namespace ResourceImporter
{

const BoundingBox&
    EditorAnimation::getModelBounds(const BoundingBoxModel& boundingBoxModel) const
{
    if (modelBoundsID) {
        return boundingBoxModel.getBoundingBox(modelBoundsID).modelBounds;
    }
    else {
        return customModelBounds;
    }
}

const EditorSprite* EditorAnimation::getSpriteAtTime(double animationTime) const
{ 
    if (frames.size() == 0) {
        return nullptr;
    }

    // Calculate which frame should be displayed at the given time.
    double frameDuration{1.0 / static_cast<double>(fps)};
    std::size_t desiredFrame{
        static_cast<std::size_t>(animationTime / frameDuration)};

    // Wrap the frame number if necessary.
    desiredFrame %= frames.size();

    // Find the sprite closest to, but not surpassing, the desired frame.
    const EditorSprite* sprite{&(frames[0].sprite.get())};
    for (std::size_t i{0}; i < (frames.size() - 1); ++i) {
        if ((frames[i].frameNumber <= desiredFrame) && 
            (frames[i + 1].frameNumber > desiredFrame)) {
            sprite = &(frames[i].sprite.get());
            break;
        }
    }

    return sprite;
}

const EditorSprite* EditorAnimation::getSpriteAtFrame(Uint8 frameNumber) const
{
    // Try to find a frame with the given number.
    for (const EditorAnimation::Frame& frame : frames) {
        if (frame.frameNumber == frameNumber) {
            return &(frame.sprite.get());
        }
    }

    // No sprite in the given frame. Return nullptr.
    return nullptr;
}

} // End namespace ResourceImporter
} // End namespace AM
