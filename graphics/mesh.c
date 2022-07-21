#include <GL/glew.h>

#include <soul/math/vector.h>
#include <soul/graphics/mesh.h>

const struct vec3f quad_vertices[4] = {
    vec3f(-1.0, 1.0, 0.0),
    vec3f(1.0, 1.0, 0.0),
    vec3f(1.0, -1.0, 0.0),
    vec3f(-1.0, -1.0, 0.0)
};

const struct vec3f ui_quad_vertices[4] = {
    vec3f(0.0, 0.0, 0.0),
    vec3f(1.0, 0.0, 0.0),
    vec3f(1.0, -1.0, 0.0),
    vec3f(0.0, -1.0, 0.0)
};

const struct vec2f quad_uvs[4] = {
    vec2f(0.0, 1.0),
    vec2f(1.0, 1.0),
    vec2f(1.0, 0.0),
    vec2f(0.0, 0.0)
};

const size_t quad_indices[6] = { 0, 2, 3, 0, 1, 2 };

static void cleanup_mesh(struct mesh *mesh)
{
    glDeleteBuffers(ATTRIBUTE_COUNT, mesh->vbos);
    glDeleteBuffers(1, &mesh->index_vbo);
    glDeleteVertexArrays(1, &mesh->vao);

    if (mesh->vertices)
        free(mesh->vertices);

    if (mesh->uvs)
        free(mesh->uvs);

    if (mesh->normals)
        free(mesh->normals);

    string_destroy(mesh->name);
}

static void deallocate_service(struct mesh_service *service)
{
    list_for_each (struct mesh, mesh, service->meshes) {
        cleanup_mesh(mesh);
    }

    list_destroy(&service->meshes);
}

static struct mesh *create_quad(struct mesh_service *service)
{
    struct mesh_create_info create_info = NEW_MESH_CREATE_INFO;
    create_info.name            = "quad";
    create_info.vertices        = (struct vec3f *)quad_vertices;
    create_info.uvs             = (struct vec2f *)quad_uvs;
    create_info.indices         = (size_t *)quad_indices;
    create_info.vertex_count    = 4;
    create_info.triangle_count  = 2;

    return mesh_create(service, &create_info);
}

static struct mesh *create_ui_quad(struct mesh_service *service)
{
    struct mesh_create_info create_info = NEW_MESH_CREATE_INFO;
    create_info.name            = "ui_quad";
    create_info.vertices        = (struct vec3f *)ui_quad_vertices;
    create_info.uvs             = (struct vec2f *)quad_uvs;
    create_info.indices         = (size_t *)quad_indices;
    create_info.vertex_count    = 4;
    create_info.triangle_count  = 2;

    return mesh_create(service, &create_info);
}

void mesh_service_create_resource(struct soul_instance *soul_instance)
{
    struct mesh_service *service = resource_create(
        soul_instance,
        MESH_SERVICE,
        sizeof(struct mesh_service),
        (resource_deallocator_t)&deallocate_service
    );

    list_init(&service->meshes, sizeof(struct mesh));

    service->primitives.quad    = create_quad(service);
    service->primitives.ui_quad = create_ui_quad(service);
}

static void create_vertex_objects(struct mesh *mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);
    glGenBuffers(ATTRIBUTE_COUNT, mesh->vbos);
    glGenBuffers(1, &mesh->index_vbo);
}

static void buffer_indices(struct mesh *mesh)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_vbo);
    size_t index_buffer_size = mesh->triangle_count*3*sizeof(unsigned int);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, mesh->indices, GL_STATIC_DRAW);
}

static void buffer_attribute(struct mesh *mesh, size_t vbo_index, int components, void *data)
{
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[vbo_index]);
    size_t buffer_size = mesh->vertex_count*components*sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, data, GL_STATIC_DRAW);
}

static void buffer_attributes(struct mesh *mesh)
{
    buffer_attribute(mesh, ATTRIBUTE_POSITION, 3, mesh->vertices);

    if (mesh->uvs)
        buffer_attribute(mesh, ATTRIBUTE_UV, 2, mesh->uvs);

    if (mesh->normals)
        buffer_attribute(mesh, ATTRIBUTE_NORMAL, 3, mesh->normals);
}

static void format_attribute(struct mesh *mesh, size_t vbo_index, int components)
{
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbos[vbo_index]);

    glVertexAttribPointer(
        vbo_index,
        components,
        GL_FLOAT,
        GL_FALSE,
        components*sizeof(float),
        0
    );

    glEnableVertexAttribArray(vbo_index);
}

static void format_attributes(struct mesh *mesh)
{
    format_attribute(mesh, ATTRIBUTE_POSITION, 3);

    if (mesh->uvs)
        format_attribute(mesh, ATTRIBUTE_UV, 2);

    if (mesh->normals)
        format_attribute(mesh, ATTRIBUTE_NORMAL, 3);
}

struct mesh *mesh_create(struct mesh_service *service, struct mesh_create_info *create_info)
{
    struct mesh *mesh = list_alloc(&service->meshes);

    mesh->name                  = string_create(create_info->name);
    mesh->vertices              = create_info->vertices;
    mesh->uvs                   = create_info->uvs;
    mesh->normals               = create_info->normals;
    mesh->indices               = create_info->indices;
    mesh->vertex_count          = create_info->vertex_count;
    mesh->triangle_count        = create_info->triangle_count;
    mesh->read_write_enabled    = create_info->read_write_enabled;

    create_vertex_objects(mesh);
    buffer_indices(mesh);
    buffer_attributes(mesh);
    format_attributes(mesh);

    if (!create_info->read_write_enabled) {
        mesh->vertices  = 0;
        mesh->uvs       = 0;
        mesh->normals   = 0;
    }

    return mesh;
}

void mesh_destroy(struct mesh_service *service, struct mesh *mesh)
{
    cleanup_mesh(mesh);
    list_remove(&service->meshes, mesh);
}

void mesh_draw(struct mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_vbo);
    glDrawElements(GL_TRIANGLES, mesh->triangle_count*3, GL_UNSIGNED_INT, 0);
}