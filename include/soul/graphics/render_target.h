#ifndef RENDER_TARGET_H
#define RENDER_TARGET_H

struct render_target
{
    int width;
    int height;
};

void render_target_bind(struct render_target *render_target);

#endif // RENDER_TARGET_H