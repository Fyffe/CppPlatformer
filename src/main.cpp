#include "debug_lib.h"
#include "alloc_lib.h"
#include "file_lib.h" 
#include "math_lib.h"
#include "input.h"
#include "game.h"
#include "platform.h"

#define APIENTRY
#define GL_GLEXT_PROTOTYPES
#include <glcorearb.h>

static KeyCodeID keyCodeLUT[KEY_COUNT];

#if _WIN32
#include "win32_platform.cpp"
#endif

#include "gl_renderer.cpp"

typedef decltype(update_game) update_game_type;
static update_game_type* update_game_ptr;

void reload_game_dll(BumpAllocator* transientStorage);

int main()
{
    BumpAllocator transientStorage = make_bump_allocator(MB(50));
    BumpAllocator persistenStorage = make_bump_allocator(MB(50));

    input = (Input*)bump_alloc(&persistenStorage, sizeof(Input));

    if(!input)
    {
        _ERROR("Failed to allocate Input");
        
        return -1;
    }

    renderData = (RenderData*)bump_alloc(&persistenStorage, sizeof(RenderData));

    if(!renderData)
    {
        _ERROR("Failed to allocate RenderData");
        
        return -1;
    }

    gameState = (GameState*)bump_alloc(&persistenStorage, sizeof(GameState));

    if(!gameState)
    {
        _ERROR("Failed to allocate GameState");

        return -1;
    }

    platform_create_keycode_lut();
    platform_create_window(1280, 640, "Platformer Game");
    gl_init(&transientStorage);

    while(isRunning)
    {
        reload_game_dll(&transientStorage);
        platform_update_window();
        update_game(gameState, renderData, input);
        gl_render();
        platform_swap_buffers();

        transientStorage.used = 0;
    }

    return 0;
}

void update_game(GameState* inGameState, RenderData* inRenderData, Input* inInput)
{
    update_game_ptr(inGameState, inRenderData, inInput);
}

void reload_game_dll(BumpAllocator* transientStorage)
{
    static void* gameDll;
    static long long lastEditTimestamp;
    
    long long currentTimestamp = get_timestamp("game.dll");

    if(currentTimestamp > lastEditTimestamp)
    {
        if(gameDll)
        {
            bool freeResult = platform_free_dynamic_library(gameDll);

            _ASSERT(freeResult, "Failed to free game.dll");

            gameDll = nullptr;

            _TRACE("Freed game.dll");
        }

        while(!copy_file("game.dll", "game_load.dll", transientStorage))
        {
            Sleep(10);
        }

        _TRACE("Copied game.dll in to game_load.dll");

        gameDll = platform_load_dynamic_library("game_load.dll");

        _ASSERT(gameDll, "Failed to load game_load.dll");

        update_game_ptr = (update_game_type*)platform_load_dynamic_function(gameDll, "update_game");

        _ASSERT(update_game_ptr, "Failed to load update_game function from game.dll");

        lastEditTimestamp = currentTimestamp;
    }
}