#ifndef VECTOR_H__
#define VECTOR_H__

#include <stddef.h>

typedef struct {
    void* elements;
    size_t element_size;
    size_t count;
    size_t capacity;
}
Vector;

void vector_init(Vector* const vector, size_t const element_size);
void vector_destroy(Vector* const vector);

void* vector_at(Vector* const vector, size_t const index);
void* vector_append(Vector* const vector);

int vector_reserve(Vector* const vector, size_t const new_capacity);
int vector_resize(Vector* const vector, size_t const new_count);
void vector_shrink(Vector* const vector);

#endif /* VECTOR_H__ */
