#include <GL/glew.h>

#include <soul/core.h>
#include <soul/debug.h>
#include <soul/graphics/core.h>
#include <soul/graphics/texture.h>
#include <soul/graphics/shader.h>
#include <soul/ui/window.h>

static void service_deallocate(struct graphics_service *service) { }

static void init_gl(struct soul_instance *soul_instance)
{
    GLenum r = glewInit();

    if (r != GLEW_OK) {
        debug_log(
            SEVERITY_ERROR,
            "Failed to create graphics_service.\nCould not initialize GLEW.\nError code %d.\n",
            r
        );

        abort();
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void graphics_service_create_resource(struct soul_instance *soul_instance)
{
    struct graphics_service *service = resource_create(
        soul_instance,
        GRAPHICS_SERVICE,
        sizeof(struct graphics_service),
        (resource_deallocator_t)&service_deallocate
    );

    init_gl(soul_instance);
}

void graphics_set_clear_color(struct vec4f color)
{
    glClearColor(color.x, color.y, color.z, color.w);
}

void graphics_clear(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}