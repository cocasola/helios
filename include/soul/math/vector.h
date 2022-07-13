#ifndef VECTOR_H
#define VECTOR_H

struct vec4f
{
    float x;
    float y;
    float z;
    float w;
} __attribute__((packed));

#define vec4f(x, y, z, w) ((struct vec4f){ (x), (y), (z), (w) })

#define VEC4F_ONE   vec4f(1, 1, 1, 1)
#define VEC4F_ZERO  vec4f(0, 0, 0, 0)

struct vec3f
{
    float x;
    float y;
    float z;
} __attribute__((packed));

#define vec3f(x, y, z) ((struct vec3f){ (x), (y), (z) })

#define VEC3F_ONE   vec3f(1, 1, 1)
#define VEC3F_ZERO  vec3f(0, 0, 0)

struct vec2f
{
    float x;
    float y;
} __attribute__((packed));

#define vec2f(x, y) ((struct vec2f){ (x), (y) })

#define VEC2F_ONE   vec2f(1, 1)
#define VEC2F_ZERO  vec2f(0, 0)

struct vec2i
{
    int x;
    int y;
} __attribute__((packed));

#define vec2i(x, y) ((struct vec2i){ (x), (y) })

#define VEC2I_ONE   vec2i(1, 1)
#define VEC2I_ZERO  vec2i(0, 0)

#endif // VECTOR_H