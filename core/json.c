#include <soul/json.h>
#include <soul/debug.h>

struct parser
{
    const char *            cursor;
    const char *            json;
    struct json_element *   current_element;
};

static char error_line_buffer[64];

static bool_t seek_symbol(struct parser *parser)
{
    while (TRUE) {
        switch (*parser->cursor) {
            case '\0':
                return FALSE;

            case ' ':
                break;

            case '\n':
                break;

            case '\r':
                break;

            default:
                return TRUE;
        }

        ++parser->cursor;
    }
}

static void fill_error_line_buffer(struct parser *parser)
{
    while (parser->cursor > parser->json) { 
        if (*parser->cursor == '\n' || *parser->cursor == '\r') {
            ++parser->cursor;

            break;
        }

        --parser->cursor;
    }

    memset(error_line_buffer, 0, 64);

    for (int i = 0; i < 63; ++i) {
        if (parser->cursor[i] == '\n' ||
            parser->cursor[i] == '\r' ||
            parser->cursor[i] == '\0')
            break;

        error_line_buffer[i] = parser->cursor[i];
    }
}

#define error(parser, message) do {         \
    debug_log(                              \
        SEVERITY_WARNING,                   \
        "Failed to parse JSON. %s\n\t>%s",  \
        message,                            \
        error_line_buffer                   \
    );                                      \
} while (0)

static bool_t is_letter(char c)
{
    return (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
}

static bool_t is_number(char c)
{
    return c >= 48 && c <= 57;
}

static bool_t is_valid_string_char(char c)
{
    return c >= 33 && c <= 126;
}

static struct string get_string(struct parser *parser)
{
    ++parser->cursor;

    char string_buffer[64];
    memset(string_buffer, 0, 64);

    for (int i = 0; i < 63; ++i) {
        if (*parser->cursor == '\"') {
            return string_create(string_buffer);
        } else if (!is_valid_string_char(*parser->cursor)) {
            error(parser, "Invalid token.");

            return (struct string){ 0, 0 };
        }

        string_buffer[i] = *(parser->cursor++);
    }

    return string_create(string_buffer);
}

static void *allocate_element(struct parser *parser, size_t size, struct string name)
{
    struct json_element *element = calloc(1, size);

    element->parent  = parser->current_element;
    element->name    = name;

    return element;
}

static struct json_object *process_object(struct parser *parser, struct string name);
static struct json_string *process_string(struct parser *parser, struct string name);
static struct json_array *process_array(struct parser *parser, struct string name);
static struct json_number *process_number(struct parser *parser, struct string name);

static struct json_element *process_named_element(struct parser *parser)
{
    struct string name = get_string(parser);

    if (!name.chars)
        return 0;

    ++parser->cursor;

    if (!seek_symbol(parser)) {
        error(parser, "Expected symbol.");
        return 0;
    }

    if (*parser->cursor != ':') {
        error(parser, "Unexpected symbol.");
        return 0;
    }

    ++parser->cursor;

    if (!seek_symbol(parser)) {
        error(parser, "Expected symbol.");
        return 0;
    }

    switch (*parser->cursor) {
        case '\"':
            return (struct json_element *)process_string(parser, name);
            break;

        case '{':
            return (struct json_element *)process_object(parser, name);
            break;

        case '[':
            return (struct json_element *)process_array(parser, name);
            break;
    }

    if (is_number(*parser->cursor))
        return (struct json_element *)process_number(parser, name);

    error(parser, "Unexpected symbol.");

    return 0;
}

static struct json_element *process_anonymous_element(struct parser *parser)
{
    bool_t found = seek_symbol(parser);

    if (!found) {
        error(parser, "Expected symbol.");
        return 0;
    }

    switch (*parser->cursor) {
        case '{':
            return (struct json_element *)process_object(parser, string_create(""));

        case '\"':
            return (struct json_element *)process_string(parser, string_create(""));

        case '[':
            return (struct json_element *)process_array(parser, string_create(""));

        case '.':
            return (struct json_element *)process_number(parser, string_create(""));
    }

    if (is_number(*parser->cursor))
        return (struct json_element *)process_number(parser, string_create(""));

    error(parser, "Invalid symbol.");

    return 0;
}

static struct json_object *process_object(struct parser *parser, struct string name)
{
    struct json_object *object = allocate_element(parser, sizeof(struct json_object), name);
    object->type = JSON_OBJECT;

    string_map_init(&object->children, sizeof(struct json_element *));

    parser->current_element = (struct json_element *)object;

    ++parser->cursor;

    if (!seek_symbol(parser)) {
        error(parser, "expected symbol.");

        return 0;
    }

    if (*parser->cursor != '}') {
        while (TRUE) {
            if (*parser->cursor != '"') {
                error(parser, "expected symbol.");

                return 0;
            }

            struct json_element *element = process_named_element(parser);

            if (!element)
                return 0;

            string_map_insert(&object->children, element->name.chars, &element);

            if (!seek_symbol(parser)) {
                error(parser, "expected symbol.");

                return 0;
            }

            if (*parser->cursor == '}') {
                ++parser->cursor;

                break;
            } else if (*parser->cursor == ',') {
                ++parser->cursor;
            } else {
                error(parser, "Unexpected symbol.");

                return 0;
            }

            if (!seek_symbol(parser)) {
                error(parser, "expected symbol.");

                return 0;
            }
        }
    }

    parser->current_element = object->parent;

    return object;
}

static struct json_string *process_string(struct parser *parser, struct string name)
{
    struct json_string *string = allocate_element(parser, sizeof(struct json_string), name);
    string->type = JSON_STRING;

    string->string = get_string(parser);

    if (!string->string.chars)
        return 0;

    ++parser->cursor;

    return string;
}

static struct json_array *process_array(struct parser *parser, struct string name)
{
    struct json_array *array = allocate_element(parser, sizeof(struct json_array), name);
    array->type = JSON_ARRAY;

    list_init(&array->elements, sizeof(struct json_element *));

    parser->current_element = (struct json_element *)array;

    ++parser->cursor;

    if (!seek_symbol(parser)) {
        error(parser, "expected symbol.");

        return 0;
    }

    if (*parser->cursor != ']') {
        while (TRUE) {
            struct json_element *element = process_anonymous_element(parser);

            if (!element)
                return 0;

            list_push(&array->elements, &element);

            if (!seek_symbol(parser)) {
                error(parser, "expected symbol.");

                return 0;
            }

            if (*parser->cursor == ']') {
                ++parser->cursor;

                break;
            } else if (*parser->cursor == ',') {
                ++parser->cursor;
            } else {
                error(parser, "Unexpected symbol.");

                return 0;
            }

            if (!seek_symbol(parser)) {
                error(parser, "expected symbol.");

                return 0;
            }
        }
    }

    parser->current_element = array->parent;

    return array;
}

static struct json_number *process_number(struct parser *parser, struct string name)
{
    struct json_number *number = allocate_element(parser, sizeof(struct json_number), name);
    number->type = JSON_NUMBER;

    char string_buffer[64];
    memset(string_buffer, 0, 63);

    for (int i = 0; i < 63; ++i) {
        if (is_number(*parser->cursor)) {
            string_buffer[i] = *parser->cursor;
        } else if (*parser->cursor == '.') {
            string_buffer[i] = *parser->cursor;

            if (number->type == JSON_NUMBER_DECIMAL) {
                error(parser, "Unexpected token.");

                return 0;
            }

            number->number_type = JSON_NUMBER_DECIMAL;
        } else {
           break; 
        }

        ++parser->cursor;
    }

    if (number->number_type == JSON_NUMBER_INTEGER)
        sscanf(string_buffer, "%d", &number->integer);
    else
        sscanf(string_buffer, "%f", &number->decimal);

    return number;
}

struct json_object *json_parse_string(const char *json)
{
    struct parser parser = {
        .cursor             = json,
        .json               = json,
        .current_element    = 0
    };

    struct json_element *element = process_anonymous_element(&parser);

    if (element->type != JSON_OBJECT) {
        error(&parser, "Expected anonymous JSON object.");

        return 0;
    }

    return (struct json_object *)element;
}

static void destroy_object(struct json_object *object);
static void destroy_array(struct json_array *array);
static void destroy_string(struct json_string *string);
static void destroy_number(struct json_number *number);

void destroy_element(struct json_element *element)
{
    switch (element->type) {
        case JSON_OBJECT:
            destroy_object((struct json_object *)element);
            break;

        case JSON_ARRAY:
            destroy_array((struct json_array *)element);
            break;

        case JSON_STRING:
            destroy_string((struct json_string *)element);
            break;

        case JSON_NUMBER:
            destroy_number((struct json_number *)element);
            break;
    }
}

static void destroy_object(struct json_object *object)
{
    list_for_each (struct json_element *, p_child, object->children.values) {
        destroy_element(*p_child);
    }

    string_map_destroy(&object->children);
    string_destroy(object->name);

    free(object);
}

static void destroy_array(struct json_array *array)
{
    list_for_each (struct json_element *, p_child, array->elements) {
        destroy_element(*p_child);
    }

    list_destroy(&array->elements);
    string_destroy(array->name);

    free(array);
}

static void destroy_string(struct json_string *string)
{
    string_destroy(string->name);
    string_destroy(string->string);

    free(string);
}

static void destroy_number(struct json_number *number)
{
    string_destroy(number->name);

    free(number);
}

void json_destroy_object(struct json_object *object)
{
    destroy_object(object);

    if (object->parent) {
        debug_log(SEVERITY_ERROR, "Do not call json_destroy_object() on non root object.\n");
        abort();
    }
}

static void print_name(struct string name)
{
    if (name.length)
        printf("\"%s\": ", name.chars);
}

static void print_tabs(int depth)
{
    for (int i = 0; i < depth; ++i) {
        printf("    ");
    }
}

static void print_object(struct json_object *object, int depth);
static void print_array(struct json_array *array, int depth);
static void print_string(struct json_string *string, int depth);
static void print_number(struct json_number *number, int depth);

static void print_element(struct json_element *element, int depth)
{
    switch (element->type) {
        case JSON_OBJECT:
            print_object((struct json_object *)element, depth);
            break;

        case JSON_ARRAY:
            print_array((struct json_array *)element, depth);
            break;

        case JSON_STRING:
            print_string((struct json_string *)element, depth);
            break;

        case JSON_NUMBER:
            print_number((struct json_number *)element, depth);
            break;
    }
}

static void print_object(struct json_object *object, int depth)
{
    print_tabs(depth);

    print_name(object->name);
    printf("{\n");

    list_for_each (struct json_element *, p_child, object->children.values) {
        print_element(*p_child, depth + 1);

        if (list_get_next(p_child))
            printf(",");

        printf("\n");
    }

    print_tabs(depth);
    printf("}");
}

static void print_array(struct json_array *array, int depth)
{
    print_tabs(depth);

    print_name(array->name);
    printf("[\n");

    list_for_each (struct json_element *, p_child, array->elements) {
        print_element(*p_child, depth + 1);

        if (list_get_next(p_child))
            printf(",");

        printf("\n");
    }

    print_tabs(depth);
    printf("]");
}

static void print_string(struct json_string *string, int depth)
{
    print_tabs(depth);

    print_name(string->name);

    printf("\"%s\"", string->string.chars);
}

static void print_number(struct json_number *number, int depth)
{
    print_tabs(depth);

    print_name(number->name);

    if (number->number_type == JSON_NUMBER_INTEGER)
        printf("%d", number->integer);
    else
        printf("%f", number->decimal);
}

void json_print_object(struct json_object *object)
{
    print_object(object, 0);
}