#ifndef USER_INPUT_H
#define USER_INPUT_H

#include "../typedefs.h"
#include "../callbacks.h"
#include "../math/vector.h"
#include "virtual_key.h"

#define USER_INPUT_MOUSE_BUTTON_COUNT    3
#define USER_INPUT_KEY_COUNT            128

typedef int mouse_button_t;
#define MOUSE_LEFT   0
#define MOUSE_RIGHT  1
#define MOUSE_MIDDLE 2

struct mouse_state
{
    struct vec2i    position;
    struct vec2i    delta;
    bool_t          buttons[USER_INPUT_MOUSE_BUTTON_COUNT];
};

struct user_input
{
    struct mouse_state  mouse;
    bool_t              keyboard[USER_INPUT_KEY_COUNT];
    struct list         on_mouse_move; // struct callback, struct window *
    struct list         on_left_click; // struct callback, struct window *
};

void user_input_init(struct user_input *input);
void user_input_destroy(struct user_input *input);

#endif // USER_INPUT_H