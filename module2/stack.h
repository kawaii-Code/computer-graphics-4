#ifndef COMPUTER_GRAPHICS_4_STACK_H
#define COMPUTER_GRAPHICS_4_STACK_H

#include "vector.h"

#define stack(type) struct stack_##type { vector_ptr(type) v; int top; }

#define stack_init(st) do { vector_init((st.v)); _vector_grow(st.v, 4); st.top = -1; } while (0)
#define stack_init_from_vec(st, vec) do { (st).v = (vec); (st).top = (vec).len; } while (0)

#define stack_push(st, item) do { \
        if ((st).top + 1 < (st).v.len) { \
            _vector_grow((st).v, 1); \
            (st).v.head[(st).top + 1] = item; \
        } else { \
            vector_append((st).v, item); \
        } \
        (st).top++; \
    } while (0)

#define stack_pop(st) \
    ( ((st).top < 0) ? (fprintf(stderr, "Stack is empty (len=%zu)\n", (size_t)(st).v.len), \
    exit(EXIT_FAILURE), vector_get((st).v, 0)) : \
    ( (st).top--, vector_get((st).v, (st).top + 1)) )

#define stack_top(st) \
    ( ((st).top < 0) ? (fprintf(stderr, "Stack is empty (len=%zu)\n", (size_t)(st).v.len), \
    exit(EXIT_FAILURE), vector_get((st).v, 0)) : \
    ( vector_get((st).v, (st).top)) )

#endif //COMPUTER_GRAPHICS_4_STACK_H