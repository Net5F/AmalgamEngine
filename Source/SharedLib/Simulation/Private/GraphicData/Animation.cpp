#include "Animation.h"
#include "Sprite.h"
#include "SharedConfig.h"
#include "Log.h"

namespace AM
{

const Sprite* Animation::getSpriteAtTime(double animationTime) const
{ 
    // Note: ResourceImporter should guarantee that every animation has at 
    //       least one filled frame.

    // Calculate which frame should be displayed, based on the given time and 
    // the animation's looping behavior.
    std::size_t desiredFrame{};
    double frameDuration{1.0 / static_cast<double>(fps)};
    if (loopStartFrame == frameCount) {
        // Play once
        desiredFrame = static_cast<std::size_t>(
            std::round(animationTime / frameDuration));
        if (desiredFrame >= frameCount) {
            return nullptr;
        }
    }
    else {
        // Loop

        // If we haven't looped yet, calc the frame as normal.
        double playOnceTime{frameCount * frameDuration};
        if (animationTime < playOnceTime) {
            desiredFrame = static_cast<std::size_t>(
                std::floor(animationTime / frameDuration));
        }
        else {
            // We've looped. Subtract the first play and calc the loop 
            // remainder.
            int loopFrameCount{frameCount - loopStartFrame};
            double loopTime{loopFrameCount * frameDuration};
            double remainderLoopTime{
                std::fmod((animationTime - playOnceTime), loopTime)};
            desiredFrame = loopStartFrame
                           + static_cast<std::size_t>(
                               std::floor(remainderLoopTime / frameDuration));
        }
    }

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

    return sprite;
}

double Animation::getLengthS() const
{
    return frameCount / static_cast<double>(fps);
}

Uint32 Animation::getLengthTicks() const
{
    double lengthS{getLengthS()};
    return static_cast<Uint32>(lengthS / SharedConfig::SIM_TICK_TIMESTEP_S);
}

} // End namespace AM
