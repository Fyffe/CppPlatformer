#version 430 core

struct Transform
{
    vec2 position;
    vec2 size;
    ivec2 atlasOffset;
    ivec2 spriteSize;
};

layout(std430, binding = 0) buffer TransformSBO
{
    Transform transforms[];
};

uniform vec2 screenSize;
uniform mat4 orthoProj;

layout (location = 0) out vec2 textureCoordsOut;

void main()
{
    Transform transform = transforms[gl_InstanceID];

    vec2 vertices[6] = 
    {
        transform.position,
        vec2(transform.position + vec2(0.0, transform.size.y)),
        vec2(transform.position + vec2(transform.size.x, 0.0)),
        vec2(transform.position + vec2(transform.size.x, 0.0)),
        vec2(transform.position + vec2(0.0, transform.size.y)),
        transform.position + transform.size
    };

    float left = transform.atlasOffset.x;
    float top = transform.atlasOffset.y;
    float right = transform.atlasOffset.x + transform.spriteSize.x;
    float bottom = transform.atlasOffset.y + transform.spriteSize.y;

    vec2 textureCoords[6] =
    {
        vec2(left, top),
        vec2(left, bottom),
        vec2(right, top),
        vec2(right, top),
        vec2(left, bottom),
        vec2(right, bottom)
    };

    {
        vec2 vertexPos = vertices[gl_VertexID];
        gl_Position = orthoProj * vec4(vertexPos, 0.0, 1.0);
    }

    textureCoordsOut = textureCoords[gl_VertexID];
}