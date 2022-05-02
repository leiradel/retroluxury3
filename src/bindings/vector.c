#include "vector.h"

#include <stdlib.h>
#include <stdint.h>

void vector_init(Vector* const vector, size_t const element_size) {
    vector->elements = NULL;
    vector->element_size = element_size;
    vector->count = 0;
    vector->capacity = 0;
}

void vector_destroy(Vector* const vector) {
    free(vector->elements);
    vector_init(vector, vector->element_size);
}

void* vector_at(Vector* const vector, size_t const index) {
    if (index < 0 || index >= vector->count) {
        return NULL;
    }

    return (uint8_t*)vector->elements + index * vector->element_size;
}

void* vector_append(Vector* const vector) {
    if (vector_reserve(vector, vector->count + 1) != 0) {
        return NULL;
    }

    return vector_at(vector, vector->count++);
}

int vector_reserve(Vector* const vector, size_t const new_capacity) {
    size_t new_cap_power_2 = 8;

    while (new_cap_power_2 < new_capacity) {
        new_cap_power_2 *= 2;
    }

    if (new_cap_power_2 < vector->capacity) {
        return 0;
    }

    size_t new_size = new_cap_power_2 * vector->element_size;
    void* const new_elements = realloc(vector->elements, new_size);

    if (new_elements == NULL) {
        return -1;
    }

    vector->elements = new_elements;
    vector->capacity = new_cap_power_2;
    return 0;
}

int vector_resize(Vector* const vector, size_t const new_count) {
    if (new_count > vector->capacity) {
        if (vector_reserve(vector, new_count) != 0) {
            return -1;
        }
    }
    else if (new_count < vector->capacity) {
        vector->elements = realloc(vector->elements, new_count * vector->element_size);
        vector->capacity = new_count;
    }

    vector->count = new_count;
    return 0;
}

void vector_shrink(Vector* const vector) {
    vector_resize(vector, vector->count);
}
