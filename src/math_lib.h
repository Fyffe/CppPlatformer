#pragma once

struct Vec2
{
    float x, y;

    Vec2 operator+(Vec2 v)
    {
        return {x + v.x, y + v.y};
    }

    Vec2 operator-(Vec2 v)
    {
        return {x - v.x, y - v.y};
    }

    Vec2 operator/(float scalar)
    {
        return {x / scalar, y / scalar};
    }

    Vec2 operator*(float scalar)
    {
        return {x * scalar, y * scalar};
    }

    Vec2 operator+=(Vec2 v)
    {
        this->x += v.x;
        this->y += v.y;
        
        return *this;
    }

    Vec2 operator-=(Vec2 v)
    {
        this->x -= v.x;
        this->y -= v.y;
        
        return *this;
    }
};

struct IVec2
{
    int x, y;

    IVec2 operator+(IVec2 v)
    {
        return {x + v.x, y + v.y};
    }

    IVec2 operator-(IVec2 v)
    {
        return {x - v.x, y - v.y};
    }

    IVec2 operator/(int scalar)
    {
        return {x / scalar, y / scalar};
    }

    IVec2 operator*(int scalar)
    {
        return {x * scalar, y * scalar};
    }

    IVec2 operator+=(IVec2 v)
    {
        this->x += v.x;
        this->y += v.y;
        
        return *this;
    }

    IVec2 operator-=(IVec2 v)
    {
        this->x -= v.x;
        this->y -= v.y;
        
        return *this;
    }
};

Vec2 vec2(IVec2 v)
{
    return Vec2
    {
        v.x * 1.0f, 
        v.y * 1.0f
    };
}

struct Vec4
{
    union
    {
        float values[4];

        struct
        {
            float x, y, z, w;
        };

        struct
        {
            float r, g, b, a;
        };
    };

    float& operator[](int index)
    {
        return values[index];
    }
};

struct Matrix4x4
{
    union
    {
        Vec4 values[4];
        struct
        {
            float ax;
            float bx;
            float cx;
            float dx;

            float ay;
            float by;
            float cy;
            float dy;

            float az;
            float bz;
            float cz;
            float dz;

            float aw;
            float bw;
            float cw;
            float dw;
        };
    };

    Vec4& operator[](int column)
    {
        return values[column];
    }
};


Matrix4x4 orthographic_projection(float left, float right, float top, float bottom)
{
    static float far = 1.0f;
    static float near = 0.0f;

    Matrix4x4 result = {};

    result.aw = -(right + left) / (right - left);
    result.bw = (top + bottom) / (top - bottom);
    result.cw = near;

    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = 1.0f / (far - near);
    result[3][3] = 1.0f;

    return result;
}