#include <soul/components.h>
#include <soul/graphics/sprite.h>
#include <soul/graphics/camera.h>
#include <soul/ui/ui_canvas.h>
#include <soul/ui/ui_container.h>

void components_register_all(struct soul_instance *soul_instance)
{
    camera_register_component(soul_instance);
    sprite_register_component(soul_instance);
    ui_canvas_register_component(soul_instance);
    ui_container_register_component(soul_instance);
}