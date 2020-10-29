#pragma once

#include <SDL_stdinc.h>
#include <array>

/**
 * This file contains shared game definitions that should be
 * consistent between the server and client.
 */
namespace AM
{
static constexpr unsigned int GAME_TICKS_PER_SECOND = 30;
static constexpr double GAME_TICK_TIMESTEP_S = 1.0
/ static_cast<double>(GAME_TICKS_PER_SECOND);

/** Game constants. */
static constexpr unsigned int MAX_ENTITIES = 200;

// TODO: Move these out.
static constexpr unsigned int SCREEN_WIDTH = 1280;
static constexpr unsigned int SCREEN_HEIGHT = 720;

/** Represents a single entity. Will be reused if the entity is destructed. */
typedef Uint32 EntityID;

/** Structs. */
struct ComponentFlag {
    enum FlagType {
        Position = 1 << 0,
        Movement = 1 << 1,
        Input = 1 << 2,
        Sprite = 1 << 3,
        Client = 1 << 4
    };
};

struct Position {
    float x = 0.0;
    float y = 0.0;
};

struct Input {
    enum Type {
        None,
        Up,
        Left,
        Down,
        Right,
        Exit, // Exit the application.
        NumTypes
    };

    enum State : Uint8 { Released, Pressed };

    Type type = Type::None;
    State state = State::Released;
};

typedef std::array<Input::State, Input::Type::NumTypes> InputStateArr;

} /* End namespace AM */
