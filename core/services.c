#include <soul/services.h>
#include <soul/ecs.h>
#include <soul/ui/window.h>
#include <soul/graphics/graphics.h>

void services_init_all(struct soul_instance *soul_instance)
{
    ecs_service_init_resource(soul_instance);
    window_service_init_resource(soul_instance);
    graphics_service_init_resource(soul_instance);
}