#ifndef MATRIX_H
#define MATRIX_H

#include "vector.h"

struct mat4x4
{
    float m00; float m01; float m02; float m03; 
    float m04; float m05; float m06; float m07; 
    float m08; float m09; float m10; float m11; 
    float m12; float m13; float m14; float m15; 
};

#define mat4x4(m00, m01, m02, m03,                      \
               m04, m05, m06, m07,                      \
               m08, m09, m10, m11,                      \
               m12, m13, m14, m15)  ((struct mat4x4){   \
    m00, m01, m02, m03,                                 \
    m04, m05, m06, m07,                                 \
    m08, m09, m10, m11,                                 \
    m12, m13, m14, m15                                  \
})

#define MAT4X4_IDENTITY mat4x4(     \
    1, 0, 0, 0,                     \
    0, 1, 0, 0,                     \
    0, 0, 1, 0,                     \
    0, 0, 0, 1                      \
)

struct mat4x4   mul_4x4(struct mat4x4 *a, struct mat4x4 *b);
void            set_pos_4x4(struct mat4x4 *m, struct vec3f pos);
void            set_scale_4x4(struct mat4x4 *m, struct vec3f scale);
void            set_rot_4x4(struct mat4x4 *m, struct vec3f rot);

#endif // MATRIX_H