#include "djb2.h"

djb2_hash djb2(char const* str) {
    djb2_hash hash = 5381;

    while (*str != 0) {
        hash = hash * 33 + (uint8_t)*str++;
    }

    return hash;
}
