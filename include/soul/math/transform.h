#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "vector.h"

struct transform
{
    struct vec3f position;
    struct vec3f rotation;
    struct vec3f scale;
};

#endif // TRANSFORM_H