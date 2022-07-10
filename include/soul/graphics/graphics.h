#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../math/vector.h"

#define GRAPHICS_SERVICE "graphics_service"

struct graphics_service
{

};

void graphics_service_init_resource(struct soul_instance *soul_instance);
void graphics_set_clear_color(struct vec4f color);
void graphics_clear(void);

#endif // GRAPHICS_H