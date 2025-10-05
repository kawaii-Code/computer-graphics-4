#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define VECTOR_TYPE(type) struct vector_##type

#define vector(type) struct vector_##type {type *head; size_t len; size_t cap; }

#define vector_new() {0,0,0}
#define vector_init(vec) do {(vec).head=0; (vec).len=0; (vec).cap=0;} while(0)

#define vector_free(vec) do {free((vec).head);} while(0)

#define vector_get(vec, i) \
    ( (((i) < (vec).len) && ((i) >= 0)) ? (vec).head[(i)] : \
    (fprintf(stderr, "Vector index %zu out of bounds (len=%zu)\n", (size_t)(i), (vec).len), \
    exit(EXIT_FAILURE), \
    (vec).head[0]) )

#define vector_get_ptr(vec, i) \
    ( (((i) < (vec).len) && ((i) >= 0)) ? &(vec).head[(i)] : \
    (fprintf(stderr, "Vector index %zu out of bounds (len=%zu)\n", (size_t)(i), (vec).len), \
    exit(EXIT_FAILURE), \
    &(vec).head[0]) )

#define vector_reserve(vec, room) do { \
        size_t newCap = (vec).cap+(room); \
        if ((vec).cap<newCap) \
            _vector_realloc(vec, newCap); \
        (vec).head+(vec).len; \
    } while(0)

//#define vector_append(vec, ...) _vector_append(vec, typeof((*(vec).head)), __VA_ARGS__)
//#define _vector_append(vec, type, ...) do { \
//        type src_[] = {__VA_ARGS__}; \
//        size_t count_ = sizeof(src_)/sizeof(*src_); \
//        if (count_) { \
//            size_t len_ = (vec).len; \
//            _vector_grow(vec, len_ + count_); \
//            memcpy((vec).head + len_, src_, count_ * sizeof(*(vec).head)); \
//            (vec).len += count_; \
//        } \
//    } while(0)

#define vector_append(vec, value) do { \
        size_t len_ = (vec).len; \
        _vector_grow(vec, len_ + 1); \
        (vec).head[len_] = (value); \
        (vec).len++; \
    } while(0)

#define _vector_grow(vec, need) do { \
        size_t need_ = (need); \
        if (need_ > (vec).cap) \
            _vector_realloc(vec, _vector_next_alloc((vec).cap, need_)); \
    } while(0)

#define _vector_realloc(vec, newCap) do { \
        (vec).cap = newCap; \
        (vec).head = realloc((vec).head, (vec).cap * sizeof(*(vec).head)); \
    } while(0)

static inline size_t _vector_next_alloc(size_t cap, const size_t need) {
    if (cap == 0) cap = 1;
    while (cap < need) cap *= 2;

    return cap;
}

#endif //VECTOR_H
