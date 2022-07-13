#include <soul/components.h>
#include <soul/graphics/sprite.h>

void components_register_all(struct soul_instance *soul_instance)
{
    sprite_register_component(soul_instance);
}