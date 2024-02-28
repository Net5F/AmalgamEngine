#include "EditorAnimation.h"
#include "BoundingBoxModel.h"

namespace AM
{
namespace ResourceImporter
{

void EditorAnimation::setFrame(Uint8 frameNumber, const EditorSprite& sprite)
{
    // If a frame matches the given number, overwrite it.
    auto insertIt{frames.end()};
    for (auto it = frames.begin(); it != frames.end(); ++it) {
        if (it->frameNumber == frameNumber) {
            it->sprite = sprite;
            break;
        }
        else if (it->frameNumber > frameNumber) {
            // Number is higher than desired, insert the new frame in front of 
            // this one.
            insertIt = it;
            break;
        }
    }

    // No match, insert a new frame.
    frames.insert(insertIt, {frameNumber, sprite});
}

void EditorAnimation::clearFrame(Uint8 frameNumber)
{
    // If a frame matches the given number, erase it.
    for (auto it = frames.begin(); it != frames.end(); ++it) {
        if (it->frameNumber == frameNumber) {
            frames.erase(it);
            break;
        }
    }
}

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

const EditorSprite& EditorAnimation::getSpriteAtTime(double animationTime) const
{ 
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

    return *sprite;
}

} // End namespace ResourceImporter
} // End namespace AM
