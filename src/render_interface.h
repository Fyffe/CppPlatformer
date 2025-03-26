#pragma once

#include "math_lib.h"
#include "assets.h"

constexpr int MAX_TRANSFORMS = 1000;

struct Transform
{
    Vec2 position;
    Vec2 size;
    IVec2 atlasOffset;
    IVec2 spriteSize;
};

struct OrthoCamera2D
{
    Vec2 position;
    Vec2 dimensions;
    float zoom = 1.0f;
};

struct RenderData
{
    OrthoCamera2D mainCamera;
    OrthoCamera2D uiCamera;
    int transformCount;
    Transform transforms[MAX_TRANSFORMS];
};

static RenderData* renderData;

Vec2 screen_to_world(IVec2 screenPosition)
{
    OrthoCamera2D camera = renderData->mainCamera;

    float x = (screenPosition.x * 1.0f) / input->screenSize.x * camera.dimensions.x;
    float y = (screenPosition.y * 1.0f) / input->screenSize.y * camera.dimensions.y;

    x += -camera.dimensions.x * 0.5f + camera.position.x;
    y += -camera.dimensions.y * 0.5f - camera.position.y;

    return {x, y};
}

IVec2 screen_to_world_pp(IVec2 screenPosition)
{
    OrthoCamera2D camera = renderData->mainCamera;

    int x = (screenPosition.x * 1.0f) / (input->screenSize.x * 1.0f) * camera.dimensions.x;
    int y = (screenPosition.y * 1.0f) / (input->screenSize.y * 1.0f) * camera.dimensions.y;

    x += -camera.dimensions.x * 0.5f + camera.position.x;
    y += camera.dimensions.y * 0.5f + camera.position.y;

    return {x, y};
}


void draw_sprite(SpriteID spriteID, Vec2 position)
{
    Sprite sprite = get_sprite(spriteID);

    Transform transform = {};
    transform.atlasOffset = sprite.atlasOffset;
    transform.spriteSize = sprite.spriteSize;
    transform.position = position - vec2(sprite.spriteSize) * 0.5f;
    transform.size = vec2(sprite.spriteSize);

    renderData->transforms[renderData->transformCount++] = transform;
}

void draw_sprite(SpriteID spriteID, IVec2 position)
{
    draw_sprite(spriteID, vec2(position));
}