#include <soul/services.h>
#include <soul/ecs.h>
#include <soul/ui/window.h>
#include <soul/graphics/core.h>
#include <soul/graphics/shader.h>
#include <soul/graphics/texture.h>
#include <soul/graphics/mesh.h>

void services_create_all(struct soul_instance *soul_instance)
{
    ecs_service_create_resource(soul_instance);
    window_service_create_resource(soul_instance);
    graphics_service_create_resource(soul_instance);
    shader_service_create_resource(soul_instance);
    texture_service_create_resource(soul_instance);
    mesh_service_create_resource(soul_instance);
}