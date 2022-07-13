#include <math.h>

#include <soul/math/matrix.h>

struct mat4x4 mul_4x4(struct mat4x4 *a, struct mat4x4 *b)
{
    struct mat4x4 r;

    r.m00 = a->m00*b->m00 + a->m01*b->m04 + a->m02*b->m08 + a->m03*b->m12;
    r.m01 = a->m00*b->m01 + a->m01*b->m05 + a->m02*b->m09 + a->m03*b->m13;
    r.m02 = a->m00*b->m02 + a->m01*b->m06 + a->m02*b->m10 + a->m03*b->m14;
    r.m03 = a->m00*b->m03 + a->m01*b->m07 + a->m02*b->m11 + a->m03*b->m15;

    r.m04 = a->m04*b->m00 + a->m05*b->m04 + a->m06*b->m08 + a->m07*b->m12;
    r.m05 = a->m04*b->m01 + a->m05*b->m05 + a->m06*b->m09 + a->m07*b->m13;
    r.m06 = a->m04*b->m02 + a->m05*b->m06 + a->m06*b->m10 + a->m07*b->m14;
    r.m07 = a->m04*b->m03 + a->m05*b->m07 + a->m06*b->m11 + a->m07*b->m15;

    r.m08 = a->m08*b->m00 + a->m09*b->m04 + a->m10*b->m08 + a->m11*b->m12;
    r.m09 = a->m08*b->m01 + a->m09*b->m05 + a->m10*b->m09 + a->m11*b->m13;
    r.m10 = a->m08*b->m02 + a->m09*b->m06 + a->m10*b->m10 + a->m11*b->m14;
    r.m11 = a->m08*b->m03 + a->m09*b->m07 + a->m10*b->m11 + a->m11*b->m15;

    r.m12 = a->m12*b->m00 + a->m13*b->m04 + a->m14*b->m08 + a->m15*b->m12;
    r.m13 = a->m12*b->m01 + a->m13*b->m05 + a->m14*b->m09 + a->m15*b->m13;
    r.m14 = a->m12*b->m02 + a->m13*b->m06 + a->m14*b->m10 + a->m15*b->m14;
    r.m15 = a->m12*b->m03 + a->m13*b->m07 + a->m14*b->m11 + a->m15*b->m15;

    return r;
}

void set_pos_4x4(struct mat4x4 *m, struct vec3f pos)
{
    m->m03 = pos.x;
    m->m07 = pos.y;
    m->m11 = pos.z;
}

void set_scale_4x4(struct mat4x4 *m, struct vec3f scale)
{
    m->m00 = scale.x;
    m->m05 = scale.y;
    m->m10 = scale.z;
}

void set_rot_4x4(struct mat4x4 *m, struct vec3f rot)
{
    struct mat4x4 tmp;

    tmp = mat4x4(
        cos(rot.y), 0, sin(rot.y), 0,
        0, 1, 0, 0,
        -sin(rot.y), 0, cos(rot.y), 0,
        0, 0, 0, 1
    );

    *m = mul_4x4(&tmp, m);

    tmp = mat4x4(
        1, 0, 0, 0,
        0, cos(rot.x), sin(rot.x), 0,
        0, -sin(rot.x), cos(rot.x), 0,
        0, 0, 0, 1
    );

    *m = mul_4x4(&tmp, m);

    tmp = mat4x4(
        cos(rot.z), -sin(rot.z), 0, 0,
        sin(rot.z), cos(rot.z), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    *m = mul_4x4(&tmp, m);
}