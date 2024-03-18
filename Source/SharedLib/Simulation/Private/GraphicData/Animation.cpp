#include "Animation.h"
#include "Sprite.h"
#include "Log.h"

namespace AM
{

const Sprite& Animation::getSpriteAtTime(double animationTime) const
{ 
    // Calculate which frame should be displayed at the given time.
    double frameDuration{1.0 / static_cast<double>(fps)};
    std::size_t desiredFrame{
        static_cast<std::size_t>(std::round(animationTime / frameDuration))};

    // Wrap the frame number if necessary.
    desiredFrame %= frameCount;

    // Find the sprite closest to, but not surpassing, the desired frame.
    const Sprite* sprite{&(frames[0].sprite.get())};
    for (std::size_t i{0}; i < frames.size(); ++i) {
        // If this frame number matches, or the next frame is a higher number, 
        // return this frame's sprite.
        if ((frames[i].frameNumber == desiredFrame)
            || (((i + 1) < frames.size())
                && (frames[i + 1].frameNumber > desiredFrame))) {
            sprite = &(frames[i].sprite.get());
            break;
        }
    }

    return *sprite;
}

} // End namespace AM
