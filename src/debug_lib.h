#pragma once

#include <stdio.h>

#ifdef _WIN32

#define DEBUG_BREAK() __debugbreak()

#elif __linux__

#define DEBUG_BREAK() __builtin_debugtrap()

#elif __APPLE__

#define DEBUG_BREAK() __builtin_trap()

#endif

enum TextColor
{
    TEXT_COLOR_BLACK,
    TEXT_COLOR_RED,
    TEXT_COLOR_GREEN,
    TEXT_COLOR_YELLOW,
    TEXT_COLOR_WHITE,
    TEXT_COLOR_COUNT
};

template <typename ...Args>
void _log(char* prefix, char* msg, TextColor color, Args... args)
{
    static char* TextColorTable[TEXT_COLOR_COUNT] =
    {
        "\x1b[30m",
        "\x1b[31m",
        "\x1b[32m",
        "\x1b[33m",
        "\x1b[37m",
    };

    char formatBuffer[8192] = {};

    sprintf(formatBuffer, "%s %s %s \033[0m", TextColorTable[color], prefix, msg);

    char textBuffer[8192] = {};

    sprintf(textBuffer, formatBuffer, args...);

    puts(textBuffer);
}

#define _TRACE(msg, ...) _log("TRACE: ", msg, TEXT_COLOR_WHITE, ##__VA_ARGS__);
#define _WARN(msg, ...) _log("WARN: ", msg, TEXT_COLOR_YELLOW, ##__VA_ARGS__);
#define _ERROR(msg, ...) _log("ERROR: ", msg, TEXT_COLOR_RED, ##__VA_ARGS__);

#define _ASSERT(x, msg, ...)            \
{                                       \
    if(!(x))                            \
    {                                   \
        _ERROR(msg, ##__VA_ARGS__);     \
        DEBUG_BREAK();                  \
    }                                   \
}                                           