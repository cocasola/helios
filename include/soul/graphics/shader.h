#ifndef SHADER_H
#define SHADER_H

#include "../string.h"
#include "../list.h"
#include "../math/matrix.h"
#include "../math/vector.h"

#define SHADER_SERVICE "shader_service"

typedef int uniform_t;

struct shader
{
    struct string   name;
    unsigned int    shader_program;
    unsigned int    vertex_shader;
    unsigned int    fragment_shader;
};

struct shader_defaults
{
    struct shader *sprite;
    struct shader *ui;
};

struct shader_service
{
    struct list             shaders; // struct shader
    struct shader_defaults  defaults;
};

void            shader_service_create_resource(struct soul_instance *soul_instance);
struct shader * shader_create(struct shader_service *shader_service,
                              const char *name,
                              const char *vertex_path,
                              const char *fragment_path);
void            shader_destroy(struct shader_service *shader_service, struct shader *shader);
void            shader_uniform_mat4x4(uniform_t uniform, struct mat4x4 *value);
void            shader_uniform_vec4f(uniform_t uniform, struct vec4f value);
void            shader_uniform_int(uniform_t uniform, int value);
uniform_t       shader_get_uniform(struct shader *shader, const char *name);
void            shader_bind(struct shader *shader);

#endif // SHADER_H