#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

typedef int ui_type_t;

#define UI_ELEMENT ui_type_t            type;   \
                   int                  depth;  \
                   struct ui_container *parent;

#include "../typedefs.h"
#include "../ecs.h"
#include "ui_container.h"

typedef int ui_type_t;
#define UI_TYPE_CONTAINER   0

struct ui_element
{
    UI_ELEMENT
};

void ui_element_link_ancestry(struct ui_element *ui_element,
                              struct entity *entity,
                              struct ecs_service *ecs);

#endif // UI_ELEMENT_H