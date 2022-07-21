#ifndef GRAPHICS_CORE_H
#define GRAPHICS_CORE_H

#include "../list.h"
#include "../core.h"
#include "../math/vector.h"

#define GRAPHICS_SERVICE "graphics_service"

struct graphics_service {};

void graphics_service_create_resource(struct soul_instance *soul_instance);
void graphics_set_clear_colour(struct vec4f colour);
void graphics_clear(void);

#endif // GRAPHICS_CORE_H