#include "debug_lib.h"
#include "alloc_lib.h"
#include "file_lib.h" 
#include "input.h"
#include "platform.h"

#define APIENTRY
#define GL_GLEXT_PROTOTYPES

#include "glcorearb.h"

#if _WIN32
#include "win32_platform.cpp"
#endif

#include "gl_renderer.cpp"

int main()
{
    BumpAllocator transientStorage = make_bump_allocator(MB(50));

    input.screenWidth = 1280;
    input.screenHeight = 720;

    platform_create_window(input.screenWidth, input.screenHeight, "Platformer Game");
    gl_init(&transientStorage);

    while(isRunning)
    {
        platform_update_window();
        gl_render();
        platform_swap_buffers();
    }

    return 0;
}