#include <soul/components.h>
#include <soul/graphics/sprite.h>
#include <soul/graphics/camera.h>

void components_register_all(struct soul_instance *soul_instance)
{
    camera_register_component(soul_instance);
    sprite_register_component(soul_instance);
}