#pragma once

static bool isRunning = true;

bool platform_create_window(int width, int height, char* title);
void platform_update_window();
void* platform_load_gl_function(char* functionName);
void platform_swap_buffers();