#pragma once

#include "platforms_lib.h"
#include "types_lib.h"
#include "math_lib.h"
#include "input.h"
#include "render_interface.h"

enum GameInputType
{
    MOVE_LEFT,
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_UP,
    JUMP,
    MOUSE_PRIMARY,
    MOUSE_SECONDARY,

    GAME_INPUT_COUNT
};

struct KeyMapping
{
    Array<KeyCodeID, 3> keys;
};

struct GameState
{
    bool isInitialized = false;
    IVec2 playerPosition;
    KeyMapping keyMappings[GAME_INPUT_COUNT];
};

static GameState* gameState;

extern "C"
{
    EXPORT_FN void update_game(GameState* inGameState, RenderData* inRenderData, Input* inInput);
}