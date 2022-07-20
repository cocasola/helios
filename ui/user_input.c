#include <soul/ui/user_input.h>
#include <soul/ui/window.h>

void user_input_init(struct user_input *input)
{
    list_init(&input->on_mouse_move, sizeof(struct callback));
    list_init(&input->on_left_click, sizeof(struct callback));
}

void user_input_destroy(struct user_input *input)
{
    list_destroy(&input->on_mouse_move);
    list_destroy(&input->on_left_click);
}