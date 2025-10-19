#ifndef COMPUTER_GRAPHICS_4_LLIST_H
#define COMPUTER_GRAPHICS_4_LLIST_H

#define LLIST_TYPE(type) struct llist_##type
#define _NODE_TYPE(type) struct _node_##type

#define DEFINE_NODE(type) \
    typedef struct _node_##type { \
        type data; \
        struct _node_##type *next; \
        struct _node_##type *prev; \
    } _node_##type;

#define _node(type) struct _node_##type { type data; _node_##type* next; _node_##type* prev; }
#define llist(type) struct llist_##type { _node(type)* head; }

#define llist_init(lst) do { (lst).head = 0; } while (0)

#define llist_add_after(lst, type, value, after) do { \
        _NODE_TYPE(type)* cur = (lst).head; \
        size_t end = after; \
        for (size_t i = 0; i < end; i++) { \
            if (!(cur)->next) { \
                fprintf(stderr, "At is bigger than llist %d %d\n", i, end); \
                exit(EXIT_FAILURE); \
            } \
            cur = cur->next; \
        } \
        _NODE_TYPE(type)* newNode = malloc(sizeof(_NODE_TYPE(type))); \
        newNode->data = (value); \
        if (cur) { \
            newNode->next = cur->next; \
            newNode->prev = cur; \
            if (cur->next) cur->next->prev = newNode; \
            cur->next = newNode; \
        } else { \
            newNode->next = (lst).head; \
            newNode->prev = (lst).head; \
            (lst).head = newNode; \
        } \
    } while (0)

#endif //COMPUTER_GRAPHICS_4_LLIST_H