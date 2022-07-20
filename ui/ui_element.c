#include <soul/ui/ui_element.h>

void ui_element_link_ancestry(struct ui_element *element,
                              struct entity *entity,
                              struct ecs_service *ecs)
{
    struct ui_container *parent_container = component_get_storage(
        ecs,
        entity->parent,
        UI_CONTAINER
    ).passive;

    if (parent_container) {
        list_push(&parent_container->children, &element);
        element->parent = parent_container;
    }
}