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
    // Try to find a sprite to display.
    for (auto it{frames.rbegin()}; it != frames.rend(); ++it) {
        if (it->frameNumber == frameNumber) {
            // Found the desired frame.
            return &(it->sprite.get());
        }
        else if (it->frameNumber < frameNumber) {
            // The desired frame wasn't found, but a previous frame with a 
            // sprite was found.
            return &(it->sprite.get());
        }
    }

    // No sprite found in the desired frame or a previous frame. Return nullptr.
    return nullptr;
}

std::vector<const EditorSprite*> EditorAnimation::getExpandedFrameVector() const
{
    std::vector<const EditorSprite*> expandedFrameVector(frameCount, nullptr);
    for (const EditorAnimation::Frame& frame : frames) {
        expandedFrameVector.at(frame.frameNumber) = &(frame.sprite.get());
    }

    return expandedFrameVector;
}

void EditorAnimation::setFromExpandedFrameVector(
    const std::vector<const EditorSprite*>& expandedFrameVector)
{
    frames.clear();

    Uint8 frameNumber{0};
    for (const EditorSprite* frame : expandedFrameVector) {
        if (frame) {
            frames.emplace_back(frameNumber, *frame);
        }

        frameNumber++;
    }

    frameCount = static_cast<Uint8>(expandedFrameVector.size());
}

} // End namespace ResourceImporter
} // End namespace AM
