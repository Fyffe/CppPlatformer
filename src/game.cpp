#include "game.h"
#include "assets.h"

constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILE_SIZE = 8;

bool is_just_pressed(GameInputType inputType)
{
    KeyMapping key = gameState->keyMappings[inputType];

    for(int index = 0; index < key.keys.count; index++)
    {
        if(input->keys[key.keys[index]].justPressed)
        {
            return true;
        }
    }

    return false;
}

bool is_down(GameInputType inputType)
{
    KeyMapping key = gameState->keyMappings[inputType];

    for(int index = 0; index < key.keys.count; index++)
    {
        if(input->keys[key.keys[index]].isDown)
        {
            return true;
        }
    }

    return false;
}

EXPORT_FN void update_game(GameState* inGameState, RenderData* inRenderData, Input* inInput)
{
    if(gameState != inGameState)
    {
        gameState = inGameState;
    }

    if(renderData != inRenderData)
    {
        renderData = inRenderData;
    }

    if(input != inInput)
    {
        input = inInput;
    }

    if(!inGameState->isInitialized)
    {
        renderData->mainCamera.dimensions = {WORLD_WIDTH, WORLD_HEIGHT};
        inGameState->isInitialized = true;

        gameState->keyMappings[MOVE_LEFT].keys.add(KEY_A);
        gameState->keyMappings[MOVE_DOWN].keys.add(KEY_S);
        gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_D);
        gameState->keyMappings[MOVE_UP].keys.add(KEY_W);
        gameState->keyMappings[MOVE_LEFT].keys.add(KEY_LEFT);
        gameState->keyMappings[MOVE_DOWN].keys.add(KEY_DOWN);
        gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_RIGHT);
        gameState->keyMappings[MOVE_UP].keys.add(KEY_UP);
    }

    draw_sprite(SPRITE_BOX, gameState->playerPosition);

    if(is_down(MOVE_LEFT))
    {
        gameState->playerPosition += {-1, 0};
    }
    if(is_down(MOVE_DOWN))
    {
        gameState->playerPosition += {0, 1};
    }
    if(is_down(MOVE_RIGHT))
    {
        gameState->playerPosition += {1, 0};
    }
    if(is_down(MOVE_UP))
    {
        gameState->playerPosition += {0, -1};
    }

    // for(int y = 0; y < 24; y++)
    // {
    //     for(int x = 0; x < 24; x++)
    //     {
    //         draw_sprite(SPRITE_BOX, {x * 64.0f, y * 64.0f});
    //     }
    // }
}