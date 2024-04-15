#pragma once

#include <string>
#include <variant>

namespace AM
{

/** Used as a "we should never hit this" cap on the text lengths. */
static constexpr std::size_t MAX_DIALOGUE_LINE_TEXT_LENGTH{500};

/**
 * Describes a piece of dialogue that was said.
 */
struct SayEvent
{
    std::string text{};
};

/**
 * Describes anything that isn't verbal: actions, settings, thoughts, feelings.
 */
struct NarrateEvent
{
    std::string text{};
};

/**
 * Describes how long to wait, in seconds, before handling the next event.
 */
struct WaitEvent
{
    float waitTimeS{1};
};

/** A dialogue event. */
using DialogueEvent = std::variant<SayEvent, NarrateEvent, WaitEvent>;


template<typename S>
void serialize(S& serializer, SayEvent& sayEvent)
{
    serializer.text1b(sayEvent.text, MAX_DIALOGUE_LINE_TEXT_LENGTH);
}

template<typename S>
void serialize(S& serializer, NarrateEvent& narrateEvent)
{
    serializer.text1b(narrateEvent.text, MAX_DIALOGUE_LINE_TEXT_LENGTH);
}

template<typename S>
void serialize(S& serializer, WaitEvent& waitEvent)
{
    serializer.value4b(waitEvent.waitTimeS);
}

} // namespace AM
