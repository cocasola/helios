#ifndef VECTOR_H
#define VECTOR_H

#include "../json.h"

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

void deserialize_vec4f(struct json_array *array, struct vec4f *destination, void *data);

struct vec3f
{
    float x;
    float y;
    float z;
} __attribute__((packed));

#define vec3f(x, y, z) ((struct vec3f){ (x), (y), (z) })

#define VEC3F_ONE   vec3f(1, 1, 1)
#define VEC3F_ZERO  vec3f(0, 0, 0)

static inline struct vec3f add3f(struct vec3f a, struct vec3f b)
{
    return vec3f(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline struct vec3f sub3f(struct vec3f a, struct vec3f b)
{
    return vec3f(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline struct vec3f mul3f_s(struct vec3f v, float scalar)
{
    return vec3f(v.x*scalar, v.y*scalar, v.z*scalar);
}

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

static inline struct vec2i add2i(struct vec2i a, struct vec2i b)
{
    return vec2i(a.x + b.x, a.y + b.y);
}

#define VEC2I_ONE   vec2i(1, 1)
#define VEC2I_ZERO  vec2i(0, 0)

#endif // VECTOR_H