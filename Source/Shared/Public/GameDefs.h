#ifndef GAMEDEFS_H
#define GAMEDEFS_H

#include <SDL_stdinc.h>
#include <array>

/**
 * This file contains shared game definitions that should be
 * consistent between the server and client.
 */
namespace AM
{
/** 30 game ticks per second. */
static constexpr float GAME_TICK_INTERVAL_S = 1 / 30.0f;

/** Game constants. */
static constexpr unsigned int MAX_ENTITIES = 100;
static constexpr unsigned int SCREEN_WIDTH = 1280;
static constexpr unsigned int SCREEN_HEIGHT = 720;

/** Represents a single entity. Will be reused if the entity is destructed. */
typedef Uint32 EntityID;

/** Structs. */
struct ComponentFlag
{
    enum FlagType
    {
        Position = 1 << 0,
        Movement = 1 << 1,
        Input = 1 << 2,
        Sprite = 1 << 3,
        Client = 1 << 4
    };
};

struct Position
{
    float x;
    float y;
};

struct Input
{
    enum Type
    {
        None,
        Up,
        Left,
        Down,
        Right,
        Exit, // Exit the application.
        NumTypes
    };

    enum State
    {
        Invalid,
        Pressed,
        Released
    };

    Input(Type inType, State inState)
    : type(inType), state(inState)
    {
    }

    Type type;
    State state;
};

typedef std::array<Input::State, Input::Type::NumTypes> InputStateArr;
struct InputSnapshot {
public:
    InputSnapshot()
    : tickNum(0)
    {
    }

    /** The tick that this snapshot was taken at. */
    Uint32 tickNum;

    /** Holds the current state of the inputs, indexed by Input::Type. */
    InputStateArr inputStates;
};

} /* End namespace AM */

#endif /* End GAMEDEFS_H */
