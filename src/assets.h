#pragma once

#include "math_lib.h"

enum SpriteID
{
    SPRITE_BOX,

    SPRITE_COUNT
};

struct Sprite
{
    IVec2 atlasOffset;
    IVec2 spriteSize;
};

Sprite get_sprite(SpriteID spriteID)
{
    Sprite sprite = {};

    switch(spriteID)
    {
        case SPRITE_BOX:
        {
            sprite.atlasOffset = {0, 0};
            sprite.spriteSize = {32, 32};
        }
    }

    return sprite;
}