#include <GL/glew.h>

#include <soul/core.h>
#include <soul/debug.h>
#include <soul/graphics/graphics.h>
#include <soul/ui/window.h>

static void deallocate_service(struct graphics_service *service) { }

static void init_gl(struct window *window)
{
    glfwMakeContextCurrent(window->glfw_handle);

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

void graphics_service_init_resource(struct soul_instance *soul_instance)
{
    struct graphics_service *service = resource_create(
        soul_instance,
        GRAPHICS_SERVICE,
        sizeof(struct graphics_service),
        (resource_deallocator_t)&deallocate_service
    );

    struct window_service *window_service = resource_get(soul_instance, WINDOW_SERVICE);

    struct window_create_info window_create_info = NEW_WINDOW_CREATE_INFO;
    window_create_info.visible                          = FALSE;
    window_create_info.hardware_acceleration_enabled    = FALSE;

    struct window *window = window_create(window_service, &window_create_info);
    init_gl(window);
    window_destroy(window_service, window);
}

void graphics_set_clear_color(struct vec4f color)
{
    glClearColor(color.x, color.y, color.z, color.w);
}

void graphics_clear(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}