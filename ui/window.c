#include <GL/glew.h>

#include <soul/resource.h>
#include <soul/execution_order.h>
#include <soul/ui/window.h>
#include <soul/graphics/core.h>

static void poll_events(struct window_service *service)
{
    glfwPollEvents();

    struct list to_destroy; // struct window *
    list_init(&to_destroy, sizeof(struct window *));

    list_for_each (struct window, window, service->windows) {
        if (glfwWindowShouldClose(window->glfw_handle)) {
            if (window->close_requested) {
                if (window->close_requested(window))
                    list_push(&to_destroy, &window);
            } else {
                list_push(&to_destroy, &window);
            }
        }
    }

    list_for_each (struct window *, p_window, to_destroy) {
        window_destroy(service, *p_window);
    }

    list_destroy(&to_destroy);
}

static void clear(struct window_service *service)
{
    list_for_each (struct window *, p_window, service->hardware_acceleration_enabled_windows) {
        window_bind(*p_window);
        graphics_set_clear_color(vec4f(0.1, 0.1, 0.12, 1.0));
        graphics_clear();
    }
}

static void swap_buffers(struct window_service *service)
{
    list_for_each (struct window *, p_window, service->hardware_acceleration_enabled_windows) {
        glfwSwapBuffers((*p_window)->glfw_handle);
    }
}

static void cleanup_window(struct window *window)
{
    string_destroy(window->title);

    list_destroy(&window->on_destroy);
    list_destroy(&window->on_resize);

    glfwDestroyWindow(window->glfw_handle);
}

static void deallocate_service(struct window_service *service)
{
    list_for_each (struct window, window, service->windows) {
        callbacks_dispatch(&window->on_destroy, window);
        cleanup_window(window);
    }

    list_destroy(&service->windows);
}

static void insert_callbacks(struct soul_instance *soul_instance, struct window_service *service)
{
    ordered_callbacks_insert(
        &soul_instance->callbacks,
        (ordered_callback_t)&poll_events,
        EXECUTION_ORDER_IO_EVENTS,
        service,
        FALSE
    );

    ordered_callbacks_insert(
        &soul_instance->callbacks,
        (ordered_callback_t)&clear,
        EXECUTION_ORDER_PRE_RENDER,
        service,
        FALSE
    );

    ordered_callbacks_insert(
        &soul_instance->callbacks,
        (ordered_callback_t)&swap_buffers,
        EXECUTION_ORDER_POST_RENDER,
        service,
        FALSE
    );
}

void window_service_create_resource(struct soul_instance *soul_instance)
{
    struct window_service *service = resource_create(
        soul_instance,
        WINDOW_SERVICE,
        sizeof(struct window_service),
        (resource_deallocator_t)&deallocate_service
    );

    list_init(&service->windows, sizeof(struct window));
    list_init(&service->hardware_acceleration_enabled_windows, sizeof(struct window *));

    insert_callbacks(soul_instance, service);

    glfwInit();

    struct window_create_info window_create_info = NEW_WINDOW_CREATE_INFO;
    service->main_window = window_create(service, &window_create_info);

    glfwMakeContextCurrent(service->main_window->glfw_handle);
}

struct window *window_create(struct window_service *service,
                             struct window_create_info *create_info)
{
    struct window *window = list_alloc(&service->windows);

    window->title                           = string_create(create_info->title);
    window->width                           = create_info->width;
    window->height                          = create_info->height;
    window->position                        = create_info->position;
    window->hardware_acceleration_enabled   = create_info->hardware_acceleration_enabled;
    window->visible                         = create_info->visible;

    list_init(&window->on_destroy, sizeof(struct callback));
    list_init(&window->on_resize, sizeof(struct callback));

    if (!window->visible)
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    else
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    window->glfw_handle = glfwCreateWindow(
        window->width,
        window->height,
        window->title.chars,
        0,
        0
    );

    if (window->hardware_acceleration_enabled)
        list_push(&service->hardware_acceleration_enabled_windows, &window);

    return window;
}

void window_destroy(struct window_service *service, struct window *window)
{
    callbacks_dispatch(&window->on_destroy, window);

    cleanup_window(window);

    list_remove(&service->windows, window);

    list_remove_value(
        &service->hardware_acceleration_enabled_windows,
        &window
    );
}

void window_bind(struct window *window)
{
    glfwMakeContextCurrent(window->glfw_handle);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window->width, window->height);
}