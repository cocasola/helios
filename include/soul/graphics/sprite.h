#ifndef SPRITE_H
#define SPRITE_H

#include "../ecs.h"
#include "../graphics/texture.h"

#define SPRITE "sprite"

struct sprite
{
    COMPONENT

    struct texture *texture;
};

void sprite_register_component(struct soul_instance *soul_instance);
void sprite_set_texture(struct sprite *sprite, struct texture *texture, bool_t match_size);

#endif // SPRITE_H