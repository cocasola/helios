#ifndef MESH_H
#define MESH_H

#include "../list.h"
#include "../string.h"
#include "../typedefs.h"
#include "../core.h"

#define MESH_SERVICE "mesh_services"

#define ATTRIBUTE_COUNT 3

#define ATTRIBUTE_POSITION  0
#define ATTRIBUTE_UV        1
#define ATTRIBUTE_NORMAL    2

struct mesh
{
    struct string   name;
    struct vec3f *  vertices;
    struct vec3f *  normals;
    struct vec2f *  uvs;
    size_t *        indices;
    int             vertex_count;
    int             triangle_count;
    unsigned int    vao;
    unsigned int    vbos[ATTRIBUTE_COUNT];
    unsigned int    index_vbo;
    bool_t          read_write_enabled;
};

struct mesh_create_info
{
    const char *    name;
    struct vec3f *  vertices;
    struct vec3f *  normals;
    struct vec2f *  uvs;
    size_t *        indices;
    int             vertex_count;
    int             triangle_count;
    bool_t          read_write_enabled;
    const char *    resource_path;
};

#define NEW_MESH_CREATE_INFO ((struct mesh_create_info){    \
    .name               = 0,                                \
    .vertices           = 0,                                \
    .normals            = 0,                                \
    .indices            = 0,                                \
    .vertex_count       = 0,                                \
    .triangle_count     = 0,                                \
    .read_write_enabled = FALSE,                            \
    .resource_path      = 0                                 \
})

struct mesh_primitives
{
    struct mesh *quad;
    struct mesh *ui_quad;
};

struct mesh_service
{
    struct list             meshes; // struct mesh
    struct mesh_primitives  primitives;
};

void            mesh_service_create_resource(struct soul_instance *soul_instance);
struct mesh *   mesh_create(struct mesh_service *service, struct mesh_create_info *create_info);
void            mesh_destroy(struct mesh_service *service, struct mesh *mesh);
void            mesh_draw(struct mesh *mesh);

#endif // MESH_H