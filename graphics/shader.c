#include <GL/glew.h>

#include <soul/resource.h>
#include <soul/file.h>
#include <soul/typedefs.h>
#include <soul/debug.h>
#include <soul/graphics/shader.h>

static void cleanup_shader(struct shader *shader)
{
    glDeleteProgram(shader->shader_program);
    string_destroy(shader->name);
}

static void service_deallocate(struct shader_service *service)
{
    list_for_each (struct shader, shader, service->shaders) {
        cleanup_shader(shader);
    }

    list_destroy(&service->shaders);
}

void shader_service_create_resource(struct soul_instance *soul_instance)
{
    struct shader_service *service = resource_create(
        soul_instance,
        SHADER_SERVICE,
        sizeof(struct shader_service),
        (resource_deallocator_t)&service_deallocate
    );

    list_init(&service->shaders, sizeof(struct shader));

    service->defaults.sprite = shader_create(
        service,
        "sprite",
        "resource/shader defaults/2d/sprite v.glsl",
        "resource/shader defaults/2d/sprite f.glsl"
    );

    service->defaults.ui = shader_create(
        service,
        "ui",
        "resource/shader defaults/2d/ui v.glsl",
        "resource/shader defaults/2d/ui f.glsl"
    );
}

static void create_shaders(struct shader *shader)
{
    shader->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    shader->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
}

static result_t compile_shader(unsigned int shader, const char *source, const char *path)
{
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);

    int compile_result = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_result);

    if (!compile_result) {
        char log[512];
        glGetShaderInfoLog(shader, 512, 0, log);
        debug_log(SEVERITY_WARNING, "Failed to compile shader \"%s\".\n\t%s\n", path, log);

        return FAIL;
    }

    return SUCCESS;
}

static void create_program(struct shader *shader)
{
    shader->shader_program = glCreateProgram();
    glAttachShader(shader->shader_program, shader->vertex_shader);
    glAttachShader(shader->shader_program, shader->fragment_shader);
}

static result_t link_program(struct shader *shader)
{
    glLinkProgram(shader->shader_program);

    int link_result = 0;
    glGetProgramiv(shader->shader_program, GL_LINK_STATUS, &link_result);

    if (!link_result) {
        char log[512];
        glGetProgramInfoLog(shader->shader_program, 512, 0, log);
        debug_log(
            SEVERITY_WARNING,
            "Failed to link program \"%s\".\n\n%s\n",
            shader->name.chars,
            log
        );

        return FAIL;
    }

    return SUCCESS;
}

static char *load_shader(const char *name,
                         const char *path)
{
    char *source = file_to_buffer(path, 0);

    if (!source) {
        debug_log(
            SEVERITY_WARNING,
            "Failed to create shader '%s'. Could not open '%s'\n",
            name,
            path
        );

        return 0;
    }

    return source;
}

static result_t load_and_compile(struct shader *shader,
                                 const char *vertex_path,
                                 const char *fragment_path)
{
    char *vertex_source = load_shader(shader->name.chars, vertex_path);
    char *fragment_source = load_shader(shader->name.chars, fragment_path);

    if (!vertex_source || !fragment_source)
        return FAIL;

    if (!compile_shader(shader->vertex_shader, vertex_source, vertex_path))
        return FAIL;

    if (!compile_shader(shader->fragment_shader, fragment_source, fragment_path))
        return FAIL;

    free(vertex_source);
    free(fragment_source);

    return SUCCESS;
}

static void cleanup_shaders(struct shader *shader)
{
    glDetachShader(shader->shader_program, shader->vertex_shader);
    glDetachShader(shader->shader_program, shader->fragment_shader);

    glDeleteShader(shader->vertex_shader);
    glDeleteShader(shader->fragment_shader);
}

struct shader *shader_create(struct shader_service *shader_service,
                             const char *name,
                             const char *vertex_path,
                             const char *fragment_path)
{
    struct shader *shader = list_alloc(&shader_service->shaders);

    shader->name = string_create(name);

    create_shaders(shader);
    create_program(shader);

    if (!load_and_compile(shader, vertex_path, fragment_path))
        return 0;

    if (!link_program(shader))
        return 0;

    cleanup_shaders(shader);

    return shader;
}

void shader_destroy(struct shader_service *shader_service, struct shader *shader)
{
    cleanup_shader(shader);
    list_remove(&shader_service->shaders, shader);
}

void shader_uniform_mat4x4(uniform_t uniform, struct mat4x4 *value)
{
    glUniformMatrix4fv(uniform, 1, FALSE, (GLfloat *)value);
}

void shader_uniform_vec4f(uniform_t uniform, struct vec4f value)
{
    glUniform4f(uniform, value.x, value.y, value.z, value.w);
}

void shader_uniform_int(uniform_t uniform, int value)
{
    glUniform1i(uniform, value);
}

uniform_t shader_get_uniform(struct shader *shader, const char *name)
{
    return glGetUniformLocation(shader->shader_program, name);
}

void shader_bind(struct shader *shader)
{
    glUseProgram(shader->shader_program);
}