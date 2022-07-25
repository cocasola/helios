#ifndef WINDOW_H
#define WINDOW_H

#include <glfw/glfw3.h>

#include "../core.h"
#include "../typedefs.h"
#include "../callbacks.h"
#include "../string.h"
#include "../callbacks.h"
#include "../math/vector.h"
#include "user_input.h"

#define WINDOW_SERVICE "window_service"

struct window_close_request
{
    bool_t          close;
    struct window * window;
};

typedef bool_t(*window_close_requested_t)(struct window *window);

struct window
{
    struct string               title;
    int                         width;
    int                         height;
    struct vec2i                position;
    struct user_input           input;
    bool_t                      hardware_acceleration_enabled;
    bool_t                      visible;
    struct list                 on_destroy; // struct callback, struct window *
    struct list                 on_resize; // struct callback, struct window *
    window_close_requested_t    close_requested;
    GLFWwindow *                glfw_handle;
};

struct window_service
{
    struct list         windows; // struct window
    struct list         hardware_acceleration_enabled_windows; // struct window *
    struct window *     main_window;
};

struct window_create_info
{
    const char *    title;
    int             width;
    int             height;
    struct vec2i    position;
    bool_t          visible;
    bool_t          hardware_acceleration_enabled;
};

#define NEW_WINDOW_CREATE_INFO ((struct window_create_info){    \
    .title                          = "soul",                   \
    .width                          = 1280,                     \
    .height                         = 800,                      \
    .position                       = vec2i(200, 200),          \
    .visible                        = TRUE,                     \
    .hardware_acceleration_enabled  = TRUE                      \
})

void            window_service_create_resource(struct soul_instance *soul_instance);
struct window * window_create(struct window_service *service,
                              struct window_create_info *create_info);
void            window_destroy(struct window_service *service, struct window *window);
void            window_bind(struct window *window);
void            window_set_size(struct window *window, int width, int height);

#endif // WINDOW_H